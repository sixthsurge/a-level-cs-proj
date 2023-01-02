#ifndef INCLUDE_CONFIG
#define INCLUDE_CONFIG

#include <fstream>
#include <map>
#include <string>
#include <vector>

// Manages the user configuration of the renderer
class Config
{
public:
    Config(const char* filePath) : m_filePath(filePath)
    {
        load();
    }

    // Load the configuration values from the file
    void load()
    {
        std::ifstream file(m_filePath, std::ios_base::in);

        for (std::string line; std::getline(file, line);)
        {
            // Find the position of first colon in the line
			auto colon = line.find_first_of(':');

            // Skip this line if it does not contain a colon
			if (colon == std::string::npos) continue;

			auto key = line.substr(0, colon);
			auto val = line.substr(colon + 1, line.length() - colon);

			m_map[key] = val;
        }

        file.close();
    }

    // Write the configuration to a file
    void save()
    {
        std::ofstream file(m_filePath, std::ios_base::out);

        for (auto pair : m_map) {
            file << pair.first;
            file << ":";
            file << pair.second;
            file << "\n";
        }

        file.close();
    }

    // Resets all configuration options
    void reset()
    {
        m_map.clear();
    }

    // Return the configuration value for a given key, or exits the program if none exists
    std::string get(std::string key)
    {
        if (m_map.count(key))
        {
            return m_map[key];
        }
        else
        {
            fmt::print("Error: required configuration variable \"{}\" not set.", key);
            std::exit(1);
        }
    }

    // Return the configuration value for a given key, or defaultVal if none exists
    std::string get(std::string key, std::string defaultVal)
    {
        if (m_map.count(key))
        {
            return m_map[key];
        }
        else
        {
            return defaultVal;
        }
    }

    int getInt(std::string key, int defaultVal = 0)
    {
        if (m_map.count(key))
        {
            auto value = m_map[key];
            
            try
            {
                return std::stoi(value);
            }
            catch (std::exception& e)
            {
                fmt::print("Configuration variable {} must be an int.\n Illegal value: {}\n", key, value);
                std::exit(1);
            }
        }
        else
        {
            return defaultVal;
        }
    }

    float getFloat(std::string key, float defaultVal = 0.0)
    {
        if (m_map.count(key))
        {
            auto value = m_map[key];
            
            try
            {
                return std::stof(value);
            } 
            catch (std::exception& e)
            {
                fmt::print("Configuration variable {} must be a float.\n Illegal value: {}\n", key, value);
                std::exit(1);
            }
        }
        else
        {
            return defaultVal;
        }
    }

    void set(std::string key, std::string value)
    {
        m_map[key] = value;
    }

    void set(std::string key, int value)
    {
        m_map[key] = std::to_string(value);
    }

    void set(std::string key, float value)
    {
        m_map[key] = std::to_string(value);
    }

private:
    std::map<std::string, std::string> m_map;
    std::string m_filePath;
};

#endif /* INCLUDE_CONFIG */
