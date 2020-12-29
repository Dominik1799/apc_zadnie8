#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

struct Image {
    std::string path;
    std::vector<int> buffer;
    int format;
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

int readHeader(Image& image) {
    std::ifstream formatRead(image.path);
    int header;
    header = formatRead.get();
    if (header != 'P')
        exit(1);
    header = formatRead.get();
    if (header - '0' > 6 || header - '0' < 1)
        exit(1);
    image.format = header - '0';
    formatRead.close();
    std::ifstream metaDataRead(image.path, std::ios_base::binary);
    metaDataRead.seekg(2, std::ios::beg);
    int byte, filePos{1};
    bool COMMENT_FLAG = false;
    std::string currentNum;
    std::vector<int> results;
    while (true) {
        byte = metaDataRead.get();
        filePos++;
        if (byte < 0) break;
        if (COMMENT_FLAG && byte != '\n') continue;
        if (COMMENT_FLAG) {
            COMMENT_FLAG = false;
            continue;
        }
        if (byte == '#') {
            COMMENT_FLAG = true;
            continue;
        }
        if (std::isspace(byte) && currentNum.empty()) continue;

        if (std::isspace(byte) && !currentNum.empty()) {
            try {
                results.push_back(std::stoi(currentNum));
                if (((image.format == 1 || image.format == 4) && results.size() == 2) || results.size() == 3)
                    break;
                currentNum.clear();
                continue;
            } catch (std::exception& e) {
                std::cerr << "Wrong header format";
                exit(1);
            }
        }
        currentNum.push_back(byte);
    }
    metaDataRead.close();
    image.width = results[0];
    image.height = results[1];
    if (results.size() == 3) {
        image.depth = results[2];
    }
    std::ifstream ppp(image.path, std::ios_base::binary);
    ppp.seekg(filePos, std::ios::beg);
    std::cout << ppp.get();
    std::cout << ppp.get();
    return filePos;
}

App parseInput(int argc, char* argv[]) {
    App app{};
    if (argc != 4)
        std::exit(1);

    if (argv[3][1] == '\0' || argv[3][0] != 'P' || argv[3][2] != '\0' || argv[3][1]-'0' > 6 || argv[3][1]-'0' < 1)
        exit(1);

    app.inputImage.path = argv[1];
    app.outputImage.path = argv[2];
    app.outputImage.format = argv[3][1] - '0';
    int dataPos = readHeader(app.inputImage);
    return app;
}






int main(int argc, char* argv[]) {
    App app = parseInput(argc, argv);
    return 0;
}
