#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

struct Image {
    std::string path;
    std::vector<int> buffer;
    size_t width;
    size_t height;
    size_t depth;
};

struct App {
    int readType;
    int writeType;
    Image inputImage;
    Image outputImage;
};

App parseInput(int argc, char* argv[]) {
    App app{};
    if (argc != 4)
        std::exit(1);

    if (argv[3][1] == '\0' || argv[3][0] != 'P' || argv[3][2] != '\0' || argv[3][1]-'0' > 6 || argv[3][1]-'0' < 1)
        exit(1);

    app.writeType = argv[3][1] - '0';
    app.inputImage.path = argv[1];
    app.outputImage.path = argv[2];
    return app;
}






int main(int argc, char* argv[]) {
    App app = parseInput(argc, argv);
    return 0;
}
