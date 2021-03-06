#ifndef LumiverseEXR_LAYER_H
#define LumiverseEXR_LAYER_H

#ifdef USE_ARNOLD_CACHING

#include <string>
#include <vector>
#include <unordered_map>
#include "Pixel.h"

namespace Lumiverse {

/**
 * Layers are defined as arrays of pixels with some modifiers.
 * Layers in lightman have RGB channels only since it does not make
 * sense for an alpha channel to exist in the illuminance space.
 */
class EXRLayer {
public:
  /**
   * Constructor.
   * Creates a new layer from an OpenEXR file.
   */
  EXRLayer(const char *file, const char *name = NULL);

  /**
   * Constructor.
   * Creates a new layer from allocated buffer.
   */
  EXRLayer(size_t w, size_t h, const char *name = NULL);

  /**
   * Destructor.
   */
  ~EXRLayer();

  /**
   * Get the name of the layer.
   */
  std::string get_name();

  /**
   * Get the width of the layer.
   */
  size_t get_width();

  /**
   * Get the height of the layer.
   */
  size_t get_height();

  /**
   * Get the number of pixels in the layer.
   */
  size_t get_size();

  /**
   * Check if the layer is active.
   */
  bool is_active();

  /**
   * Enables the layer.
   */
  void enable();

  /**
   * Disables the layer.
   */
  void disable();

  /**
   * Get the modulator of the layer.
   */
  Pixel3 get_modulator();

  /**
   * Set the modulator of the layer.
   */
  void set_modulator(Pixel3 modulator);

  /**
   * Get a pointer to the pixels.
   */
  Pixel4 *get_pixels();

  /*!
  \brief Set all bits of the buffer to 0
  */
  void clear_buffers();

  /*!
  \brief Get a downsampled image basis
  */
  Pixel4 *get_downsampled_pixels(int width, int height);

  /*!
  \brief Set the pixel buffer to the rgba value from the given buffer
  */
  void set_pixels(float *buffer);

  /*!
  \brief Set the pizel buffer to the specified Pixel4 buffer.
  EXRLayer takes ownership of this buffer.
  */
  void set_pixels(Pixel4* buffer);

private:
  /**
   * Name of the layer.
   */
  std::string name;

  /**
   * Width.
   * The width of the layer.
   */
  size_t w;

  /**
   * Height.
   * The height of the layer.
   */
  size_t h;

  /**
   * Is the layer active.
   * If a layer is not active, it is ignored in composition.
   */
  bool active;

  /**
   * Channel specific modulator of the layer.
   * A vector multiplier that does channel-specific modulation.
   */
  Pixel3 modulator;

  /**
   * Pixel buffer of the layer.
   * Do note that the pixels have RGB channels only since it does not
   * make sense for an alpha channel to exist in the illuminance space.
   */
  Pixel4 *pixels;

  /*!
   * \brief Map from an integer (width << 2 + height) key to a basis image
   * of pixels for this EXR layer
   */
  std::unordered_map<int, Pixel4*> pixel_size_bases;

  /*!
  * \brief Get the key for the size basis map from the given
  * width and height
  */
  int inline get_size_key(int width, int height);
};

}; // namespace Lumiverse

#endif // USE_ARNOLD_CACHING

#endif // LumiverseEXR_LAYER_H
