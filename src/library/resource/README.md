This library provides implementation to access a resource (data, file, image, etc.) by a key.
=============================================================================================

See ya make documentation, resources section for more details.

### Example - adding a resource file into build:
```
LIBRARY()
OWNER(user1)
RESOURCE(
    path/to/file1 /key/in/program/1
    path/to/file2 /key2
)
END()
```

### Example - access to a file content by a key:
```cpp
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/resource/resource.h>
=======
#include <src/library/resource/resource.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
int main() {
        std::cout << NResource::Find("/key/in/program/1") << std::endl;
        std::cout << NResource::Find("/key2") << std::endl;
}
```
