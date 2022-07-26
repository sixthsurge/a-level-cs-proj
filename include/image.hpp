#ifndef INCLUDE_CORE_IMAGE
#define INCLUDE_CORE_IMAGE

template <typename T>
class Image {
public:
    // Construct an uninitialized image
    Image() :
        m_data(nullptr),
        m_size(0)
    {}

    // Construct a blank image of the specified width and height
    Image(int width, int height) :
        m_size(width, height)
    {
        m_data = new T[width * height];
    }

    ~Image() {
        if (m_data != nullptr) delete [] m_data;
    }

    // Loads the image data from a png file at the specified path
    void loadFromFile(const char* path) {
        // Load the image data using the stb_image library
        int width, height, channelCount;
        char* data = stbi_load(path, &width, &height, &channelCount, T::length());

        if (data == NULL || channelCount != T::length()) {
            throw std::runtime_error(fmt::format("Failed to load image file: {}", path));
        } else {
            // Delete the old image data
            if (m_data != nullptr) delete [] m_data;

            m_data = reinterpret_cast<T*>(data);
            m_size.x = width;
            m_size.y = height;
        }
    }

    // Writes the image data to a png file at the specified path
    void writeToFile(const char* path) const {
        bool success = stbi_write_png(
			path,
			m_size.x,
			m_size.y,
			T::length(),
			reinterpret_cast<unsigned char*>(m_data),
			m_size.x * T::length()
		) != 0;

        if (!success) {
            throw std::runtime_error(fmt::format("Failed to write image file: {}", path));
        }
    }

    // Returns the data stored in the pixel at `pos`.
    T load(glm::ivec2 pos) const {
        assert(clamp(pos, glm::ivec2(0), m_size) == pos);
        int pixelIndex = getPixelIndex(pos);
        return m_data[pixelIndex];
    }

    // Stores the data in the pixel at `pos`.
    void store(glm::ivec2 pos, const T& data) {
        assert(clamp(pos, glm::ivec2(0), m_size) == pos);
        int pixelIndex = getPixelIndex(pos);
        m_data[pixelIndex] = data;
    }

    // Executes f in parallel for each pixel in the image and stores the result in that pixel. The
    // The function's parameter is the position of that pixel on the image in UV space. Using this
    // function is similar to running a fragment shader for each pixel on an image
    template <typename function>
    void process(const function& f, int groupCount) {
        assert(groupCount >= 1);

        const float pixelCount = m_size.x * m_size.y;
        const float groupSize  = (float) (pixelCount / groupCount);

        std::vector<std::thread> threads;
        threads.reserve(groupCount);

        auto processGroup = [] (Image<T>* image, const function& f, int begin, int end) {
            for (int pixelIndex = begin; pixelIndex < end; ++pixelIndex) {
                // Get the position of the pixel in the image from its index
                glm::ivec2 pos;
                pos.y = pixelIndex / image->m_size.x;
                pos.x = pixelIndex - image->m_size.x * pos.y;

                // Convert from pixel position to UV coordinates on [0.5 / res, 1.0 - 0.5 / res]
                glm::vec2 coord = (glm::vec2(pos) + 0.5f) / glm::vec2(image->m_size);

                // Call the function for this pixel
                T result = f(coord);

                // Store the result in the image
                image->m_data[pixelIndex] = result;
            }
        };

        // Start the threads
        for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
            // Calculate the indices of the first and last pixels in this group;
            const int begin = groupIndex * groupSize;
            const int end = glm::max(begin + groupSize, pixelCount);

            // Create a new thread inside of the thread list to process this group of pixels
            threads.emplace_back(processGroup, this, f, begin, end);
        }

        // Wait for all of the threads to finish
        for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
            threads[groupIndex].join();
        }
    }

private:
    // Assigns a unique integer index to each pixel, its location in the backing array
    // rows-then-columns seems to be the standard way to do this
    int getPixelIndex(glm::ivec2 pos) const {
        return pos.y * m_size.x + m_size.x;
    }

    glm::ivec2 m_size;

    T* m_data;
};

#endif /* INCLUDE_CORE_IMAGE */
