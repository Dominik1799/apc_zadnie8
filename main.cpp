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
        if (results[2] > 255){
            std::cerr << "Unsupported color depth.";
            exit(1);
        }

        image.depth = results[2];
    }
    return filePos;
}

void readData(Image& image, int dataPos) {
    std::ifstream data(image.path);
    data.seekg(dataPos, std::ios::beg);
    int byte;
    bool COMMENT_FLAG = false;
    while (true) {
        byte = data.get();
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
        if (isspace(byte)) continue;
        image.buffer.push_back(byte);
    }

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
    readData(app.inputImage, dataPos);
    return app;
}

void blackWhiteToGrayscale(App& app) {
    for (int byte : app.inputImage.buffer) {
        app.outputImage.buffer.push_back(byte);
    }
    app.outputImage.depth = 2;
}


void grayscaleToBlackWhite(App &app) {
    for (int& byte : app.inputImage.buffer) {
        if (byte <= app.inputImage.depth / 2)
            app.outputImage.buffer.push_back(1);
        else
            app.outputImage.buffer.push_back(0);
    }
}

void grayscaleToColor(App &app) {
    app.outputImage.depth = app.inputImage.depth;
    for (int byte : app.inputImage.buffer) {
        app.outputImage.buffer.push_back(byte);
        app.outputImage.buffer.push_back(byte);
        app.outputImage.buffer.push_back(byte);
    }
}

void colorToGrayscale(App &app) {
    app.outputImage.depth = app.inputImage.depth;
    for (size_t i = 0; i < (app.inputImage.height * app.inputImage.width * 3); i += 3){
        app.outputImage.buffer.push_back((app.inputImage.buffer[i] + app.inputImage.buffer[i+1] + app.inputImage.buffer[i+2]) / 3);
    }
}

void colorToBlackWhite(App &app) {
    for (size_t i = 0; i < (app.inputImage.height * app.inputImage.width * 3); i += 3){
        int temp = (app.inputImage.buffer[i] + app.inputImage.buffer[i+1] + app.inputImage.buffer[i+2]) / 3;
        if (temp <= app.inputImage.depth / 2)
            app.outputImage.buffer.push_back(1);
        else
            app.outputImage.buffer.push_back(0);
    }
}

void blackWhiteToColor(App& app) {
    app.outputImage.depth = 255;
    for (int byte : app.inputImage.buffer) {
        if (byte == 1) {
            app.outputImage.buffer.push_back(0);
            app.outputImage.buffer.push_back(0);
            app.outputImage.buffer.push_back(0);
        } else {
            app.outputImage.buffer.push_back(255);
            app.outputImage.buffer.push_back(255);
            app.outputImage.buffer.push_back(255);
        }
    }
}


void createImage(App& app) {
    app.outputImage.width = app.inputImage.width;
    app.outputImage.height = app.inputImage.height;
}






int main(int argc, char* argv[]) {
    App app = parseInput(argc, argv);
    createImage(app);
    return 0;
}
