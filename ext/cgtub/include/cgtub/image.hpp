#pragma once

#include <span>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>

namespace cgtub
{

class Image
{
public:
    Image();

    Image(unsigned int width, unsigned int height);

    Image(std::filesystem::path const& path);

    bool read(std::filesystem::path const& path);

    std::vector<glm::vec3> const& data() const;

    unsigned int width() const;

    unsigned int height() const;

    void set(unsigned int x, unsigned int y, glm::vec3 const& color);

    void set(std::span<glm::vec3 const> color);

    glm::vec3 const& get(unsigned int x, unsigned int y) const;

    glm::vec3 const& operator()(unsigned int x, unsigned int y) const;

    glm::vec3& get(unsigned int x, unsigned int y);

    glm::vec3& operator()(unsigned int x, unsigned int y);

private:
    unsigned int           m_width;
    unsigned int           m_height;
    std::vector<glm::vec3> m_data;
};

} // namespace cgtub