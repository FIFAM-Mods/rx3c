#pragma once
#include <string>
#include <vector>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

template<typename T> void Rx3SwapEndian(T &value) {}
void Rx3SwapEndian(unsigned int &value);
void Rx3SwapEndian(unsigned short &value);
void Rx3SwapEndian(int &value);
void Rx3SwapEndian(short &value);

enum Rx3SectionId : unsigned int {
    RX3_SECTION_INDEX_BUFFER = 5798132,
    RX3_SECTION_VERTEX_BUFFER = 5798561,
    RX3_SECTION_MATERIAL = 123459928,
    RX3_SECTION_BONE = 137740398,
    RX3_SECTION_NODE_NAME = 230948820,
    RX3_SECTION_PARTIAL_SKELETON = 255353250,
    RX3_SECTION_INDEX_BUFFERS_HEADER = 582139446,
    RX3_SECTION_PROP = 685399266,
    RX3_SECTION_NAMES = 1285267122,
    RX3_SECTION_TEXTURES_HEADER = 1808827868,
    RX3_SECTION_SKELETON = 1881640942,
    RX3_SECTION_TEXTURE = 2047566042,
    RX3_SECTION_NODE = 2116321516,
    RX3_SECTION_TREE_BIRD = 2360999927,
    RX3_SECTION_MORPH = 3247952176,
    RX3_SECTION_VERTEX_DECLARATION = 3263271920,
    RX3_SECTION_PRIMITIVE_TYPE = 3566041216,
    RX3_SECTION_BONE_MATRICES = 3751472158,
    RX3_SECTION_COLLISION = 4034198449,

    // 

};

enum Rx3TextureFormat {
    RX3TEXTUREFORMAT_DXT1 = 0,
    RX3TEXTUREFORMAT_DXT3 = 1,
    RX3TEXTUREFORMAT_DXT5 = 2,
    RX3TEXTUREFORMAT_L8 = 4,
    RX3TEXTUREFORMAT_ATI2 = 7, // also known as BC5 compression
    RX3TEXTUREFORMAT_ATI1 = 12, // also known as BC4 compression
};

enum Rx3TextureType {
    RX3TEXTURETYPE_2D = 1,
    RX3TEXTURETYPE_CUBEMAP = 3,
    RX3TEXTURETYPE_VOLUME = 4,
};

class Rx3Section {
public:
    friend class Rx3Container;
    unsigned int id;
    bool bigEndian;
    vector<unsigned char> data;
};

class Rx3Reader {
    unsigned char const *begin;
    unsigned char const *current;
    bool _bigEndian;
public:
    Rx3Reader(void const *data, bool bigEndian = false);
    Rx3Reader(Rx3Section *rx3section);
    Rx3Reader(Rx3Section const *rx3section);
    Rx3Reader(Rx3Section &rx3section);
    Rx3Reader(Rx3Section const &rx3section);
    size_t Position() const;
    void MoveTo(size_t position);
    void Skip(size_t bytes);
    char const *GetString();
    char const *ReadString();
    void const *GetCurrentPtr();
    void SetBigEndian(bool set);

    template<typename T>
    T const &Get() {
        static T result;
        result = *(T const *)current;
        if (_bigEndian)
            Rx3SwapEndian(result);
        return result;
    }

    template<typename T>
    T const &Read() {
        static T result;
        result = *(T const *)current;
        if (_bigEndian)
            Rx3SwapEndian(result);
        current += sizeof(T);
        return result;
    }
};

class Rx3Writer {
    vector<unsigned char> &mData;
    bool _bigEndian;

    void PutData(void const *data, unsigned int size);
public:
    Rx3Writer(vector<unsigned char> &data, bool bigEndian = false);
    Rx3Writer(Rx3Section *rx3section);
    Rx3Writer(Rx3Section const *rx3section);
    Rx3Writer(Rx3Section &rx3section);
    Rx3Writer(Rx3Section const &rx3section);
    void Put(string const &str);
    void Put(wstring const &str);
    void Put(const char *str);
    void Put(const wchar_t *str);
    void Put(void const *data, unsigned int size);
    void Align(unsigned int alignment = 16);
    void SetBigEndian(bool set);
    void Reserve(unsigned int size);

    template<typename T>
    void Put(T const &value) {
        static T result;
        result = value;
        if (_bigEndian)
            Rx3SwapEndian(result);
        PutData((void *)&result, sizeof(T));
    }
};

class Rx3Container {
public:
    vector<Rx3Section> sections;
    string name;
    bool bigEndian;

    Rx3Container();
    Rx3Container(path const &rx3path);
    bool Load(path const &rx3path);
    bool Save(path const &rx3path);
    Rx3Section *FindFirstSection(unsigned int sectionId);
    vector<Rx3Section *> FindAllSections(unsigned int sectionId);
    Rx3Section &AddSection(unsigned int sectionId);
    bool IsEmpty();
};
