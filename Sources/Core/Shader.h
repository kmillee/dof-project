#pragma once
#include <string>
#include <glm/glm.hpp>

// Simple shader class for loading and using vertex and fragment shaders
class Shader {
public:
    bool load(const std::string& vsPath, const std::string& fsPath);
    void use() const;
    void setMat4(const std::string& name, const glm::mat4& m) const;
	void setVec4(const std::string& name, const glm::vec3& v) const;
	void setVec3(const std::string& name, const glm::vec3& v) const;
	void setFloat(const std::string& name, float value) const;
	void setInt(const std::string& name, int value) const;

private:
    unsigned int m_id = 0;
};
