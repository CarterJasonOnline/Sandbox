#include <iostream>
#include <memory>
#include <vector>
#include <cstdlib>

//  ObjectWithData class that has a Reusable buffer
class ObjectWithData {
public:
    ObjectWithData( size_t id ) : id(id) {
        std::cout << "ObjectWithData " << id << " created!" << std::endl;
    }

    ~ObjectWithData() {
        std::cout << "ObjectWithData " << id << " destroyed! buffer size: " << buffer.capacity() << " Active Size :" << buffer.size() << std::endl;
    }

    std::byte* allocateBuffer(size_t size) {

        // Reallocate buffer if necessary
        buffer.resize(size);
        return buffer.data();
    }

private:
    std::vector<std::byte> buffer;
    size_t id = 0;
};