#include <iostream>
#include <memory>
#include <vector>
#include <cstdlib>

//  ObjectWithData class that has a Reusable buffer
class ObjectWithData {
public:
    ObjectWithData( size_t id ) : id(id), buffer(nullptr), bufferSize(0), activeSize(0) {
        std::cout << "ObjectWithData " << id << " created!" << std::endl;
    }

    ~ObjectWithData() {
        std::cout << "ObjectWithData " << id << " destroyed! buffer size: " << bufferSize << " Active Size :" << activeSize << std::endl;
    }

    void allocateBuffer(size_t size) {

        // Reallocate buffer if necessary
        if (bufferSize < size) {
            buffer.reset(new std::byte[size]);
            bufferSize = size;
        }
        activeSize = size;
    }

private:
    std::unique_ptr<std::byte[]> buffer;
    size_t bufferSize;
    size_t activeSize;
    size_t id;
};