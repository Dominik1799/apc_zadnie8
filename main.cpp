#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>

struct Image {
    std::string path;
    std::vector<int> buffer;
    int format;
    size_t width;
    size_t height;
    size_t depth{0};
};

struct App {
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

void readBinary(Image& image, int dataPos) {
    std::ifstream reader(image.path, std::ios::binary);
    reader.seekg(dataPos, std::ios::beg);
    size_t bitCounter{0};
    int byte;
    while (true) {
        byte = reader.get();
        if (byte < 0 && reader.eof())
            break;
        if (image.format == 4) {
            if (bitCounter > image.width) {
                for (int i = 0; i < bitCounter - image.width; i++)
                    image.buffer.pop_back();
                bitCounter = 0;
            }
            for (int i = 7; i >= 0; i--) {
                image.buffer.push_back(((byte >> i) & 1));
                bitCounter++;
            }
        } else {
            image.buffer.push_back(byte);
        }
    }
}

void readASCII(Image& image, int dataPos) {
    std::ifstream reader(image.path);
    reader.seekg(dataPos, std::ios::beg);
    int byte;
    bool COMMENT_FLAG = false;
    std::string num;

}


void readData(Image& image, int dataPos) {
    std::ifstream data(image.path);
    data.seekg(dataPos, std::ios::beg);
    int byte;
    bool COMMENT_FLAG = false;
    std::string num;
    int bruh{0};
    size_t bitCounter{0};
    while (true) {
        byte = data.get();
        if (byte < 0 && data.eof())
            break;
        bruh++;
        if (COMMENT_FLAG && byte != '\n')
            continue;
        if (COMMENT_FLAG) {
            COMMENT_FLAG = false;
            continue;
        }
        if (byte == '#' && image.format < 4) {
            COMMENT_FLAG = true;
            continue;
        }
        if (isspace(byte) && num.empty() && image.format < 4)
            continue;
        if (isspace(byte) && !num.empty()) {
            try {
                image.buffer.push_back(std::stoi(num));
                num.clear();
                continue;
            } catch (std::exception& e) {
                std::cerr << "wrong data format.";
                exit(1);
            }
        }
        if (image.format == 4) {
            if (bitCounter > image.width) {
                for (int i = 0; i < bitCounter - image.width; i++)
                    image.buffer.pop_back();
                bitCounter = 0;
            }
            for (int i = 7; i >= 0; i--) {
                image.buffer.push_back(((byte >> i) & 1));
                bitCounter++;
            }
        } else if (image.format > 4)
            image.buffer.push_back(byte);
        else if (image.format == 1)
            image.buffer.push_back(byte - '0');
        else {
            num.push_back(byte);
        }
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
    app.outputImage.depth = 1;
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
        app.outputImage.buffer.push_back(std::round((app.inputImage.buffer[i] + app.inputImage.buffer[i+1] + app.inputImage.buffer[i+2]) / 3));
    }
}

void colorToBlackWhite(App &app) {
    for (size_t i = 0; i < (app.inputImage.height * app.inputImage.width * 3); i += 3){
        int temp = std::round((app.inputImage.buffer[i] + app.inputImage.buffer[i+1] + app.inputImage.buffer[i+2]) / 3);
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
    int inType = (app.inputImage.format % 4) + (app.inputImage.format / 4);
    int outType = (app.outputImage.format % 4) + (app.outputImage.format / 4);
    if (inType == 1 && outType == 2) blackWhiteToGrayscale(app);
    else if (inType == 1 && outType == 3) blackWhiteToColor(app);
    else if (inType == 2 && outType == 1) grayscaleToBlackWhite(app);
    else if (inType == 2 && outType == 3) grayscaleToColor(app);
    else if (inType == 3 && outType == 1) colorToBlackWhite(app);
    else if (inType == 3 && outType == 2) colorToGrayscale(app);
    else if (inType == outType) {
        app.outputImage.buffer = app.inputImage.buffer;
        app.outputImage.depth = app.inputImage.depth;
    }
}

unsigned char reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

std::vector<unsigned char> createP4(Image& image) {
    std::vector<unsigned char> bytes;
    size_t dataPointer{0};
    unsigned char bitBuffer{0};
    size_t bitCounter{0};
    for (size_t height = 0; height < image.height; height++) {
        for (size_t width = 0; width < image.width; width++) {
            if (bitCounter > 7) {
                bytes.push_back(reverse(bitBuffer));
                bitBuffer = 0;
                bitCounter = 0;
            }
            bitBuffer += (image.buffer[dataPointer++] << bitCounter++) | 0;
        }
        if (bitCounter != 8)
            bytes.push_back(reverse(bitBuffer));
        bitBuffer = 0;
        bitCounter = 0;
    }

    return bytes;
}

void writeBinary(App& app) {
    std::ofstream dataWriter(app.outputImage.path, std::ios::app | std::ios::binary);
    if (app.outputImage.format == 4) {
        std::vector<unsigned char> bytes = createP4(app.outputImage);
        for (auto byte : bytes)
            dataWriter.write((char *)&byte,1);
    } else {
        for (int byte : app.outputImage.buffer) {
            dataWriter.write((char *)&byte,1);
        }
    }

}

void writeAscii(App& app) {
    std::ofstream dataWriter(app.outputImage.path, std::ios::app);
    std::string delimiter = app.outputImage.format == 1 ? "" : " ";
    for (int& data : app.outputImage.buffer) {
        dataWriter << data << delimiter;
    }
    dataWriter.close();
}


void writeImage(App& app) {
    std::ofstream headerWriter(app.outputImage.path);
    headerWriter << "P" << app.outputImage.format << std::endl;
    headerWriter << app.outputImage.width << " " << app.outputImage.height << std::endl;
    if (app.outputImage.format != 1 && app.outputImage.format != 4)
        headerWriter << app.outputImage.depth << std::endl;
    headerWriter.close();
    if (app.outputImage.format < 4)
        writeAscii(app);
    else
        writeBinary(app);

}






int main(int argc, char* argv[]) {
    App app = parseInput(argc, argv);
    std::cout << app.inputImage.buffer.size();
    createImage(app);
    writeImage(app);
    return 0;
}
