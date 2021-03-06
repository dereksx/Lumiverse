/*! \file ArnoldInterface.h
* \brief Implementation of a interface between ArnoldPatch and arnold.
*/
#ifndef _ArnoldDISTRIBUTED_INTERFACE_H_
#define _ArnoldDISTRIBUTED_INTERFACE_H_

#pragma once

#include "LumiverseCoreConfig.h"

#ifdef USE_ARNOLD
#ifdef USE_DUMIVERSE

#include <curl_easy.h>
#include <curl_exception.h>
#include <curl_form.h>
#include <zlib.h>
#include <algorithm>
#include <cstdio>
#include "ArnoldInterface.h"
#include "../lib/libjson/libjson.h"
#include <thread>
#include <iostream>
#include <locale>
#include <unordered_map>

// Apparently we need this to use zlib on windows to avoid windows being
// dumb and converting end-of-line characters for binary files
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK (1 << 17)
#define WINDOW_BITS 15
#define GZIP_ENCODING 16

namespace Lumiverse {

	/*!
	* \brief Struct containing information for storing chunked response from server
	*/
	struct RequestBuffer {
		char *buffer = NULL;
		size_t num_written = 0;
	};

	/*!
	* \brief Callback used to get bytes from curl request buffer
	*
	* When we call the render function, dumiverse will send us an array of
	* floats. In this callback we'll take the bytes in the request and set
	* them as our float *buffer.
	*
	* NOTE: According to the libcurl docs, this callback cannot be a non-static class
	* member function, and must have this exact signature
	*/
	static size_t write_buffer_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

	/*!
	* \brief Interface between ArnoldPatch and arnold. Almost all arnold APIs are called from this class.
	*
	* ArnoldInterface is mainly responsible to configure light node of arnold. It keeps a list of mappings
	* from metadata ids to Arnold types. ArnoldInterface also creates and closes Arnold session.
	* \sa ArnoldPatch
	*/
	class DistributedArnoldInterface : public virtual ArnoldInterface
	{
	public:
		/*!
		* \brief Constructs a DistributedArnoldInterface object.
		*
		* We inherit all of the normal interface fields from ArnoldInterface, so here
		* we just set the remote port and hostname
		*/
		DistributedArnoldInterface(string host, int port, string outputPath) :
			ArnoldInterface(),
			m_host_name(host),
			m_host_port(port),
			m_file_output_path(outputPath),
			m_remote_open(false) {}

		/*!
		* \brief Default DistributedArnoldInterface constructor.
		*
		* Assume that we're on localhost on port 80 by default. This will likely only
		* be used for tests.
		*/
		DistributedArnoldInterface() :
			ArnoldInterface(),
			m_host_name("localhost"),
			m_host_port(80),
			m_file_output_path("./test.out"),
			m_remote_open(false) {}


		~DistributedArnoldInterface() { }

		/*!
		* \brief Initializes a connection to the remote Arnold renderer and
		* sends it the currently loaded .ass file.
		*
		* Tries to open a connection to the remote arnold renderer host. There
		* can be at most one open connection to a distributed arnold node at any
		* given time
		*/
		void init(const JSONNode jsonPatch);

		/*!
		* \brief Close the connection with the remote arnold host.
		*
		* Closes the connection to the remote arnold host. This also causes the
		* remote arnold host to clean up and free its memory, so this should only
		* ever be done when there are no more rendering calls to be made.
		*/
		void close() override;

		/*!
		\brief Sets the dimensions of the image, and adds to settings for update on the remote renderer.
		*/
		bool setDims(int w, int h) override;
		
		/*!
		\brief Set the sampling rate used on the remote renderer
		*/
		void setSamples(int samples) override;

		/*!
		* \brief Gets the type of this object.
		*
		* \return String containing "DistributedArnoldInterface"
		*/
		virtual string getInterfaceType() { return "DistributedArnoldInterface"; }

		/*!
		* \brief Set port number on which arnold node is listening
		*
		* Set the port on which the remote arnold node host is listening
		* for requests.
		* \param hostPort Port on which the remote arnold node host is listening for requests.
		*/
		void setHostPort(long hostPort) { m_host_port = hostPort; };

		/*!
		* \brief Get the port number that remote host is listening 
		*/
		long getHostPort() { return m_host_port; };

		/*!
		* \brief Set host name of arnold node for connection over network
		*
		* Set the host name used to find the Arnold node over a network
		* \param hostName Host name of arnold node
		*/
		void setHostName(std::string hostName) { m_host_name = hostName; };

		/*!
		* \brief Get the host name of arnold an node
		*
		* Get the host name used to find the Arnold node over a 
		*/
		std::string getHostName() { return m_host_name; };

		/*!
		* \brief Perform remote Arnold rendering.
		* 
		* Returns the error code of AiRender as returned by the remote renderer, so the caller can know if the renderer was interrupted.
		* 
		* \param Set of devices that were updated since the last call to render
		* \return Error code of remote arnold renderer
		*/
		int render(const std::set<Device *> &devices);

		/*!
		* \brief Interrupts current rendering on the remote host.
		*/
		void interrupt() override;

		/*!
		* \brief Gets the progress of current frame as a percentage.
		*
		* \return The percent.
		*/
		virtual float getPercentage() override;

		/*!
		* \brief Gets the curl connection handle
		*
		* \return Curl connection handle
		*/
		curl::curl_easy getCurlConnection() { return m_curl_connection; };

		/*!
		* \brief Getter for file output path
		*
		* \return String containing file output path
		*/
		std::string getFileOutputPath() { return m_file_output_path; };

		/*!
		* \brief Setter for the file output path
		*
		* Set the path where Arnold will write the output file on the remote renderer
		*/
		void setFileOutputPath(std::string outputPath) { m_file_output_path = outputPath; };

		/*!
		* \brief Sets a parameter for the global options node in arnold to be
		* used in the remote renderer.
		*
		* Sets a parameter for the global options in arnold to be used in the
		* remote renderer. On a render call, a JSON encoding of these parameters
		* is sent over the wire and applied on the remote rendering side.
		*/
		void setOptionParameter(const std::string &paramName, int val) override;
		void setOptionParameter(const std::string &paramName, float val) override;

    /*!
    \brief returns the status of the remote connection
    */
    bool isDistributedOpen() override;

	private:

		/*!
		* \brief Hostname or IP address of the node to perform the rendering
		*/
		std::string m_host_name;

		/*!
		* \brief Port number on which remote node's arnold renderer is listening
		*/
		long m_host_port;

		/*!
		* \brief Open a connection with the remote arnold renderer host
		*
		* Opens a connection with the remote arnold host. If a connection is
		* already open this command returns false.
		*
		* \return True if connection successfully opened, false otherwise
		*/
		bool openConnection();

		/*!
		* \brief Send a POST request to the remote arnold renderer host
		* containing the gzipped .ass file relevant parameters
		*
		* Sends a POST request to the remote arnold renderer host containing
		* the gzipped .ass file, and relevant parameters it needs that are not
		* included in the .ass file such as m_gamma, m_predictive, and the
		* location that the .ass file specifies for outputting a written file
		*
		* \return True if request successful, false otherwise
		*/
		bool sendDistributedInitRequest(const JSONNode jsonPatch);

		/*!
		* \brief Close the connection with the remote arnold renderer host.
		*
		* Closes the currently open connection with the remote arnold renderer host.
		* This is necessary for the remote host to be able to receive new requests.
		*
		* \return True if connection successfully closed, false otherwise
		*/
		bool closeConnection();

		/*!
		 * \brief libcurl / curlcpp object for sending and receiving requests with the remote arnold renderer
		 */
		curl::curl_easy m_curl_connection;

		/*!
		* \brief Path to where the image will be output from arnold after being rendered.
		*
		* Path to the file on the remote endpoint where arnold will write out a bitmap.
		* This is contained in the ass file.
		*/
		std::string m_file_output_path;

		/*!
		* \brief Gzip the current assfile
		*
		* Gzip the interface's current assfile to optimize sending
		* it over the wire to the remote renderer
		*/
		bool deflateAss();

		/*!
		* \brief Bind a request buffer to a curl request for consuming the response
		*
		* By default, curl responses just send their data to stdout. Dumiverse sends JSON
		* responses, and this function binds a RequestBuffer to the request to consume
		* the response
		*/
		void bindRequestBuffer(struct RequestBuffer *buffer, curl::curl_easy &curl_request);

		/*!
		* \brief Bind a request buffer to the class' curl object
		*
		* Overloaded function for binding a request buffer to the m_curl_connection object
		* instance on this class. Note that this should never be called for getPercentage()
		* or interrupt(), as these require their own objects
		*/
		void bindRequestBuffer(struct RequestBuffer *buffer);

		/*!
		* \brief Consume a remote JSON response and determine if the request was successful.
		*/
		bool remoteRequestSuccessful(JSONNode response);

		/*!
		* \brief Get a JSON encoding of the devices being rendered on a given frame
		*
		* Get a JSON encoding of the devices being rendered on a given frame. These
		* devices are then sent over the wire to the remote renderer to be updated
		* between render calls.
		*/
		const JSONNode getDevicesJSON(const std::set<Device *> &devices);

		/*!
		* \brief JSON serialize the currently set arnold options settings
		*
		* Returns a JSON node of the currently set arnold options. This is then
		* sent over the wire to the distributed renderer on a render call.
		*/
		const JSONNode getSettingsJSON();

		// Maps containing global Arnold options node settings
		std::unordered_map<std::string, float> float_options;
		std::unordered_map<std::string, int> int_options;

		/*!
		\brief Is a connection open to the remote renderer
		*/
		bool m_remote_open;
	};
}

#endif // USE_DUMIVERSE
#endif // USE_ARNOLD
#endif // _ArnoldDISTRIBUTED_INTERFACE_H_
