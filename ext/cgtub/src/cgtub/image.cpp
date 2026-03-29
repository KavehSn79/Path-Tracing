#include "cgtub/image.hpp"

#include <iostream>
#include <cstring>

#include <stb_image.h>

namespace cgtub
{

Image::Image()
    : m_width(0)
    , m_height(0)
{
}

Image::Image(unsigned int width, unsigned int height)
    : m_width(width)
    , m_height(height)
    , m_data(width * height, glm::vec3(0))
{
}

Image::Image(std::filesystem::path const& path)
{
    read(path);
}

bool Image::read(std::filesystem::path const& path)
{
    int    width, height, n_channels;
    float* data = stbi_loadf(path.string().c_str(), &width, &height, &n_channels, 3);

    if (data == NULL)
        return false;

    m_width  = width;
    m_height = height;
    m_data.resize(m_width * m_height);

    std::memcpy(m_data.data(), data, sizeof(glm::vec3) * m_data.size());

    stbi_image_free(data);

    return true;
}

std::vector<glm::vec3> const& Image::data() const
{
    return m_data;
}

unsigned int Image::width() const
{
    return m_width;
}

unsigned int Image::height() const
{
    return m_height;
}

void Image::set(unsigned int x, unsigned int y, glm::vec3 const& color)
{
    (*this)(x, y) = color;
}

void Image::set(std::span<glm::vec3 const> color)
{
    size_t byteSize = color.size();
    if (color.size() != m_data.size())
    {
        byteSize = sizeof(glm::vec3) * std::min(m_data.size(), color.size());
        std::cerr << "[Image::set]: Input data is larger or smaller than the image data." << std::endl;
    }

    std::memcpy(m_data.data(), color.data(), byteSize);
}

glm::vec3 const& Image::get(unsigned int x, unsigned int y) const
{
    return (*this)(x, y);
}

glm::vec3 const& Image::operator()(unsigned int x, unsigned int y) const
{
    if (m_width == 0 || m_height == 0)
    {
        std::cerr << "Image::(): Image has no data (width=" << m_width << ", height=" << m_height << ")" << std::endl;
        static glm::vec3 undefined(0.f);
        return undefined;
    }

    return m_data[y * m_width + x];
}

glm::vec3& Image::get(unsigned int x, unsigned int y)
{
    return (*this)(x, y);
}

glm::vec3& Image::operator()(unsigned int x, unsigned int y)
{
    if (m_width == 0 || m_height == 0)
    {
        std::cerr << "Image::(): Image has no data (width=" << m_width << ", height=" << m_height << ")" << std::endl;
        static glm::vec3 undefined(0.f);
        return undefined;
    }

    return m_data[y * m_width + x];
}

} // namespace cgtub