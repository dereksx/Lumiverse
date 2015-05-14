
#include "ArnoldPatch.h"
#include "types/LumiverseFloat.h"
#include <sstream>

namespace Lumiverse {

#ifndef _WIN32
	void convertPlugin(std::string &dir) {
		for (size_t i = 0; i < dir.size(); i++) {
			if (dir.at(i) == ';')
				dir[i] = ':';
		}
	}
#endif

ArnoldPatch::ArnoldPatch(const JSONNode data) :
SimulationPatch() {
	loadJSON(data);
}

void ArnoldPatch::loadJSON(const JSONNode data) {
	string patchName = data.name();

	// Load options for raytracer application. (window, ray tracer, filter)
	JSONNode::const_iterator i = data.begin();

	while (i != data.end()) {
		std::string nodeName = i->name();

		if (nodeName == "jsonPath") {
			JSONNode path = *i;

			// TODO: better separator
			std::string directory = path.as_string();
			int slash;
			if ((slash = directory.find_last_of("/")) != string::npos) {
				directory = directory.substr(0, slash + 1);
			}
			else if ((slash = directory.find_last_of("\\")) != string::npos) {
				directory = directory.substr(0, slash + 1);
			}
			else
				directory += "/";
			m_interface.setDefaultPath(directory);
		}

		if (nodeName == "sceneFile") {
          JSONNode fileName = *i;
          m_interface.setAssFile(fileName.as_string());
		}
        
        if (nodeName == "pluginDir") {
            JSONNode dir = *i;
			std::string plugin = dir.as_string();
#ifndef _WIN32
			convertPlugin(plugin);
#endif
			m_interface.setPluginDirectory(plugin);
		}
        
        if (nodeName == "gamma") {
            JSONNode gamma = *i;
            m_interface.setGamma(gamma.as_float());
		}

		if (nodeName == "predictive") {
			JSONNode predictive = *i;
			m_interface.setPredictive(predictive.as_bool());
		}
        
        if (nodeName == "samples") {
            JSONNode samples = *i;
            m_interface.setSamples(samples.as_int());
		}

		if (nodeName == "lights") {
			JSONNode lights = *i;
			JSONNode::const_iterator light = lights.begin();
			while (light != lights.end()) {
				std::string light_name = light->name();

				m_lights[light_name] = new ArnoldLightRecord();
				m_lights[light_name]->metadata = light->find("type")->as_string();

				std::stringstream sstm;
				sstm << "Added light " << light_name << ": " << m_lights[light_name]->metadata;

				Logger::log(INFO, sstm.str());

				light++;
			}
		}
        
        if (nodeName == "arnoldParamMaps") {
			JSONNode params = *i;
			JSONNode::const_iterator param = params.begin();
			while (param != params.end()) {
				std::string param_name = param->name();

                m_interface.loadArnoldParam(*param);
                
                std::stringstream sstm;
                sstm << "Added param " << param_name;
                
				Logger::log(INFO, sstm.str());
                
				param++;
			}
		}

		i++;
	}

}

AtNode *ArnoldPatch::getLightNode(Device *d) {
	std::string light_name = d->getId();
	AtNode *light_ptr;

	if (m_lights.count(light_name) == 0)
		return NULL;
	std::string type = m_lights[light_name]->metadata;
	ArnoldLightRecord *record = (ArnoldLightRecord *)m_lights[light_name];
	if (record->light == NULL) {
		light_ptr = AiNode(type.c_str());
		AiNodeSetStr(light_ptr, "name", light_name.c_str());
	}
	else {
		light_ptr = record->light;
	}

	return light_ptr;
}

void ArnoldPatch::setOrientation(AtNode *light_ptr, Device *d_ptr, std::string pan_str, std::string tilt_str) {
	float pan_val;
	float tilt_val;

	std::istringstream iss_pan(pan_str);
	iss_pan >> pan_val;

	std::istringstream iss_tilt(tilt_str);
	iss_tilt >> tilt_val;

	LumiverseOrientation pan(pan_val, DEGREE, pan_val);
	LumiverseOrientation tilt(tilt_val, DEGREE, tilt_val);

	setOrientation(light_ptr, d_ptr, &pan, &tilt);
}

void ArnoldPatch::setOrientation(AtNode *light_ptr, Device *d_ptr, LumiverseOrientation *pan, LumiverseOrientation *tilt) {
	std::string lookat_str;
	std::string up_str;
	std::string pos;
	if (!d_ptr->getMetadata("lookat", lookat_str) ||
		!d_ptr->getMetadata("up", up_str) ||
		!d_ptr->getMetadata("position", pos)) {
		return ;
	}

	ArnoldParameterVector<3, float> lookat_vec;
	parseArnoldParameter<3, float>(lookat_str, lookat_vec);
	ArnoldParameterVector<3, float> up_vec;
	parseArnoldParameter<3, float>(up_str, up_vec);
	ArnoldParameterVector<3, float> pos_vec;
	parseArnoldParameter<3, float>(pos, pos_vec);

	Eigen::Vector3f lookat(lookat_vec[0] - pos_vec[0], lookat_vec[1] - pos_vec[1], lookat_vec[2] - pos_vec[2]);
	Eigen::Vector3f up(up_vec[0], up_vec[1], up_vec[2]);

	Eigen::Matrix3f rotation = LumiverseTypeUtils::getRotationMatrix(lookat, up, pan, tilt);

	std::stringstream ss;
	ss << rotation(0, 0) << "," << rotation(0, 1) << "," << rotation(0, 2) << ",0,"
		<< rotation(1, 0) << "," << rotation(1, 1) << "," << rotation(1, 2) << ",0,"
		<< rotation(2, 0) << "," << rotation(2, 1) << "," << rotation(2, 2) << ",0,"
		<< pos << ",1";

	m_interface.setParameter(light_ptr, "matrix", ss.str());
}

void ArnoldPatch::loadLight(Device *d_ptr) {
    std::string light_name = d_ptr->getId();
    AtNode *light_ptr = getLightNode(d_ptr);
    
	if (!light_ptr)
        return ;

    for (std::string meta : d_ptr->getMetadataKeyNames()) {
        std::string value;

		// Set fixed position with metadata
		// Assume we are using degree
		if (meta == "pan" && d_ptr->metadataExists("tilt")) {
			d_ptr->getMetadata(meta, value);
			std::string tilt_str;
			d_ptr->getMetadata("tilt", tilt_str);

			setOrientation(light_ptr, d_ptr, value, tilt_str);
		}
		else if (meta == "gobo" && d_ptr->metadataExists("gobo_file") &&
			d_ptr->metadataExists("degree")) {
			std::string file;
			d_ptr->getMetadata("gobo_file", file);

			std::string deg_str;
			d_ptr->getMetadata("degree", deg_str);

			float deg;

			std::istringstream iss_deg(deg_str);
			iss_deg >> deg;

			float rot = 0;
			if (d_ptr->metadataExists("gobo_rotation")) {
				std::string rot_str;
				d_ptr->getMetadata("gobo_rotation", rot_str);
				
				std::istringstream iss_rot(rot_str);
				iss_rot >> rot;
			}

			m_interface.addGobo(light_ptr, file, deg, rot);
		}
		else {
			d_ptr->getMetadata(meta, value);
			m_interface.setParameter(light_ptr, meta, value);
		}
    }
    
    // Sets arnold params with device params
    // This process is after parsing metadata, so parameters here can overwrite values from metadata
    for (std::string param : d_ptr->getParamNames()) {
      LumiverseType *raw = d_ptr->getParam(param);
      
      // First parse lumiverse type into string. So we can reuse the function for metadata.
      // It's obviously inefficient.
      if (raw->getTypeName() == "float") {
        if (param == "intensity") {
          LumiverseFloat* scaledVal = (LumiverseFloat*)LumiverseTypeUtils::copy(raw);

          if (d_ptr->metadataExists("gel")) {
            *scaledVal *= (float)(ColorUtils::getTotalTrans(d_ptr->getMetadata("gel")));
          }

          m_interface.setParameter(light_ptr, param, scaledVal->asString());
          delete scaledVal;
        }
      }
      else if (raw->getTypeName() == "color") {
          Eigen::Vector3d rgb = ((LumiverseColor*)raw)->getRGB();
          std::stringstream ss;
          ss << rgb[0] << ", " << rgb[1] << ", " << rgb[2];
          m_interface.setParameter(light_ptr, param, ss.str());
      }
      // Assume pan and tilt are named as "pan" and "tilt"
      else if (raw->getTypeName() == "orientation" &&
          param == "tilt") {
        LumiverseOrientation *tilt = (LumiverseOrientation*)raw;
        LumiverseOrientation *pan = (LumiverseOrientation*)d_ptr->getParam("pan");

        if (pan == NULL)
          continue;

        setOrientation(light_ptr, d_ptr, pan, tilt);
      }
    }

    // If there is no color parameter, use a gel color/incandescent model
    if (!d_ptr->paramExists("color")) {
      Eigen::Vector3d rgb = ColorUtils::normalizeRGB(ColorUtils::convXYZtoRGB(d_ptr->getGelColor()));
      std::stringstream ss;
      ss << rgb[0] << ", " << rgb[1] << ", " << rgb[2];
      m_interface.setParameter(light_ptr, "color", ss.str());
    }

	ArnoldLightRecord *record = (ArnoldLightRecord *)m_lights[light_name];
	record->light = light_ptr;
}

/*!
* \brief Destroys the object.
*/
ArnoldPatch::~ArnoldPatch() {
	m_interface.close();
}

void ArnoldPatch::modifyLightColor(Device *d, Eigen::Vector3d white) {
	AtNode *light_ptr = getLightNode(d);

	if (!light_ptr)
		return;

	Eigen::Vector3d rgb;
	if (d->getColor() != NULL) 
		rgb = d->getColor()->getRGB(sharpRGB);
	else {
		map<string, Eigen::Vector3d> basis;
    basis["White"] = d->getGelColor(); // Returns D65 if no gel present.
		unordered_map<string, double> channels;
		channels["White"] = 1;
		LumiverseColor white(channels, basis, ColorMode::ADDITIVE, 1);
		white.setColorChannel("White", 1);
		rgb = white.getRGB(sharpRGB);
	}

	rgb = Eigen::Vector3d(rgb[0] / white[0], rgb[1] / white[1], rgb[2] / white[2]);

	std::stringstream ss;
	ss << rgb[0] << ", " << rgb[1] << ", " << rgb[2];
	m_interface.setParameter(light_ptr, "color", ss.str());
}
    
void ArnoldPatch::updateLight(set<Device *> devices) {
	if (m_interface.getPredictive()) {
		updateLightPredictive(devices);
		return;
	}

	for (Device* d : devices) {
		std::string name = d->getId();
		if (m_lights.count(name) == 0)
			continue;
		loadLight(d);
	}
}

void ArnoldPatch::updateLightPredictive(set<Device *> devices) {
	Device *dominant = NULL;
	float max_luminant = -1;
	LumiverseColor white(BASIC_RGB);
	white.setColorChannel("Red", 1);
	white.setColorChannel("Green", 1);
	white.setColorChannel("Blue", 1);

	for (Device* d : devices) {
		std::string name = d->getId();
		if (m_lights.count(name) == 0)
			continue;
		float intensity = ((LumiverseFloat*)d->getParam("intensity"))->getVal();
		std::string exp_str = "";
		float exposure = 0.f;

		if (d->getMetadata("exposure", exp_str)) {
			std::istringstream iss(exp_str);
			iss >> exposure;
		}

		float luminant;
		float color_intensity;
		if (d->getColor() != NULL)
			color_intensity = d->getColor()->getY();
		else {
			color_intensity = 100;// white.getY();
		}

		luminant = color_intensity * intensity * powf(2, exposure);

		if (luminant > max_luminant) {
			max_luminant = luminant;
			dominant = d;
		}

		loadLight(d);
	}

	if (!dominant)
		return;

	Eigen::Vector3d rgb_w (0.9220, 1.0446, 1.0878);
	/*
	if (dominant->getColor() != NULL)
		rgb_w = dominant->getColor()->getRGB(sharpRGB);
	else {
		rgb_w = white.getRGB(sharpRGB);
	}
	*/

	for (Device* d : devices) {
		std::string name = d->getId();
		if (m_lights.count(name) == 0)
			continue;
		modifyLightColor(d, rgb_w);
	}

	m_interface.updateSurfaceColor(rgb_w);
}
    
bool ArnoldPatch::renderLoop() {
    int code = m_interface.render();

	return (code == AI_SUCCESS);
}

void ArnoldPatch::interruptRender() {
    m_interface.interrupt();
    
    if (m_renderloop != NULL) {
        try {
            m_renderloop->join();
        }
        catch (const std::system_error& e) {
            Logger::log(ERR, "Thread doesn't exist.");
        }
        m_renderloop = NULL;
    }
}
    
void ArnoldPatch::setSamples(int samples) {
    m_interface.setSamples(samples);
}
    
void ArnoldPatch::update(set<Device *> devices) {
	bool render_req = isUpdateRequired(devices);

    if (!render_req) {
        return ;
    }
    updateLight(devices);
    clearUpdateFlags();
    
    interruptRender();
    
    m_renderloop = new std::thread(&ArnoldPatch::renderLoop, this);
}

void ArnoldPatch::init() {
	// Init patch and interface
	for (auto light : m_lights) {
		m_lights[light.first]->init();
	}
	
    //m_interface.init();
}

void ArnoldPatch::close() {
	SimulationPatch::close();
    m_interface.close();
}

JSONNode ArnoldPatch::toJSON() {
	JSONNode root;

	root.push_back(JSONNode("type", getType()));
	root.push_back(JSONNode("sceneFile", m_interface.getAssFile()));
	root.push_back(JSONNode("pluginDir", m_interface.getPluginDirectory()));
	root.push_back(JSONNode("predictive", (m_interface.getPredictive()) ? 1 : 0));
	root.push_back(JSONNode("gamma", m_interface.getGamma()));

	JSONNode lights;
	lights.set_name("lights");

	for (auto light : m_lights) {
		JSONNode lightNode;
		lightNode.set_name(light.first);
		lightNode.push_back(JSONNode("type", light.second->metadata));
		lights.push_back(lightNode);
	}
	root.push_back(lights);

	root.push_back(m_interface.arnoldParameterToJSON());

	return root;
}

}
