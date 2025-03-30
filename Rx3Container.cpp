#include "Rx3Container.h"
#include "errormsg.h"
#include "memory.h"

void Rx3SwapEndian(unsigned int &value) {
    value = _byteswap_ulong(value);
}

void Rx3SwapEndian(unsigned short &value) {
    value = _byteswap_ushort(value);
}

void Rx3SwapEndian(int &value) {
    value = _byteswap_ulong(value);
}

void Rx3SwapEndian(short &value) {
    value = _byteswap_ushort(value);
}

void Rx3SwapEndian(float &value) {
    SetAt(&value, 0, _byteswap_ulong(GetAt<unsigned long>(&value, 0)));
}

Rx3Reader::Rx3Reader(void const *data, bool bigEndian) {
    begin = (unsigned char const *)data;
    current = begin;
    _bigEndian = bigEndian;
}

Rx3Reader::Rx3Reader(Rx3Section *rx3section) : Rx3Reader(rx3section->data.data(), rx3section->bigEndian) {}
Rx3Reader::Rx3Reader(Rx3Section const *rx3section) : Rx3Reader(rx3section->data.data(), rx3section->bigEndian) {}
Rx3Reader::Rx3Reader(Rx3Section &rx3section) : Rx3Reader(rx3section.data.data(), rx3section.bigEndian) {}
Rx3Reader::Rx3Reader(Rx3Section const &rx3section) : Rx3Reader(rx3section.data.data(), rx3section.bigEndian) {}

size_t Rx3Reader::Position() const {
    return current - begin;
}

void Rx3Reader::MoveTo(size_t position) {
    current = begin + position;
}

void Rx3Reader::Skip(size_t bytes) {
    current += bytes;
}

char const *Rx3Reader::GetString() {
    return (char const *)current;
}

char const *Rx3Reader::ReadString() {
    char const *result = GetString();
    current += strlen(result) + 1;
    return result;
}

void const *Rx3Reader::GetCurrentPtr() {
    return current;
}

void Rx3Reader::SetBigEndian(bool set) {
    _bigEndian = set;
}

void Rx3Writer::PutData(void const *data, unsigned int size) {
    if (size > 0) {
        unsigned int current = mData.size();
        mData.resize(current + size);
        Memory_Copy(&mData[current], data, size);
    }
}

Rx3Writer::Rx3Writer(vector<unsigned char> &data, bool bigEndian) : mData(data), _bigEndian(bigEndian) {}
Rx3Writer::Rx3Writer(Rx3Section *rx3section) : mData(rx3section->data), _bigEndian(rx3section->bigEndian) {}
Rx3Writer::Rx3Writer(Rx3Section const *rx3section) : mData(const_cast<vector<unsigned char> &>(rx3section->data)), _bigEndian(rx3section->bigEndian) {}
Rx3Writer::Rx3Writer(Rx3Section &rx3section) : mData(rx3section.data), _bigEndian(rx3section.bigEndian) {}
Rx3Writer::Rx3Writer(Rx3Section const &rx3section) : mData(const_cast<vector<unsigned char> &>(rx3section.data)), _bigEndian(rx3section.bigEndian) {}

void Rx3Writer::Put(char const *str) {
    PutData(str, strlen(str));
    Put<char>('\0');
}

void Rx3Writer::Put(wchar_t const *str) {
    PutData(str, wcslen(str) * 2);
    Put<wchar_t>('\0');
}

void Rx3Writer::Put(std::string const &str) {
    PutData((void *)str.c_str(), str.size());
    Put<char>('\0');
}

void Rx3Writer::Put(std::wstring const &str) {
    PutData((void *)str.c_str(), str.size() * 2);
    Put<wchar_t>(L'\0');
}

void Rx3Writer::Put(void const *data, unsigned int size) {
    PutData(data, size);
}

void Rx3Writer::Align(unsigned int alignment) {
    auto numBytes = GetNumBytesToAlign(mData.size(), alignment);
    for (unsigned int i = 0; i < numBytes; i++)
        Put<unsigned char>(0);
}

void Rx3Writer::SetBigEndian(bool set) {
    _bigEndian = set;
}

void Rx3Writer::Reserve(unsigned int size) {
    if (size > mData.capacity())
        mData.reserve(size);
}

Rx3Container::Rx3Container(path const &rx3path) {
    Load(rx3path);
}

bool Rx3Container::Load(path const &rx3path) {
    bool result = false;
    FILE *file = nullptr;
    _wfopen_s(&file, rx3path.c_str(), L"rb");
    if (file) {
        name = rx3path.stem().string();
        fseek(file, 0, SEEK_END);
        auto fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (fileSize >= 16) {
            unsigned char *fileData = new unsigned char[fileSize];
            fread(fileData, 1, fileSize, file);
            Rx3Reader reader(fileData);
            int fileType = 0;
            unsigned int signature = reader.Read<unsigned int>();
            if (signature == 'l3XR')
                fileType = 1;
            else if (signature == 'b3XR')
                fileType = 2;
            if (fileType == 1 || fileType == 2) {
                reader.SetBigEndian(fileType == 2);
                reader.Skip(8);
                unsigned int numSections = reader.Read<unsigned int>();
                if (numSections > 0) {
                    sections.resize(numSections);
                    for (unsigned int s = 0; s < numSections; s++) {
                        unsigned int id = reader.Read<unsigned int>();
                        unsigned int offset = reader.Read<unsigned int>();
                        unsigned int size = reader.Read<unsigned int>();
                        reader.Skip(4);
                        sections[s].id = id;
                        if (size > 0) {
                            sections[s].data.resize(size);
                            memcpy(sections[s].data.data(), &fileData[offset], size);
                        }
                    }
                }
                result = true;
            }
            delete[] fileData;
        }
        fclose(file);
    }
    return result;
}

bool Rx3Container::Save(path const &rx3path) {
    bool result = false;
    vector<unsigned char> data;
    Rx3Writer writer(data);
    if (bigEndian)
        writer.Put<unsigned int>('b3XR');
    else
        writer.Put<unsigned int>('l3XR');
    writer.SetBigEndian(bigEndian);
    writer.Put<unsigned int>(4);
    unsigned int fileHeaderSize = 16 + 16 * sections.size();
    unsigned int totalSize = fileHeaderSize;
    for (auto const &section : sections)
        totalSize += section.data.size();
    writer.Put<unsigned int>(totalSize);
    unsigned int numSections = sections.size();
    writer.Put<unsigned int>(numSections);
    unsigned int currentOffset = fileHeaderSize;
    for (auto const &section : sections) {
        writer.Put<unsigned int>(section.id);
        writer.Put<unsigned int>(currentOffset);
        writer.Put<unsigned int>(section.data.size());
        writer.Put<unsigned int>(0);
        currentOffset += section.data.size();
    }
    for (auto const &section : sections)
        writer.Put(section.data.data(), section.data.size());
    FILE *file = nullptr;
    _wfopen_s(&file, rx3path.c_str(), L"wb");
    if (file) {
        fwrite(data.data(), data.size(), 1, file);
        fclose(file);
        result = true;
    }
    return result;
}

Rx3Container::Rx3Container() {

}

Rx3Section *Rx3Container::FindFirstSection(unsigned int sectionId) {
    for (auto &s : sections) {
        if (s.id == sectionId)
            return &s;
    }
    return nullptr;
}

vector<Rx3Section *> Rx3Container::FindAllSections(unsigned int sectionId) {
    vector<Rx3Section *> result;
    for (auto &s : sections) {
        if (s.id == sectionId)
            result.push_back(&s);
    }
    return result;
}

Rx3Section &Rx3Container::AddSection(unsigned int sectionId) {
    Rx3Section section;
    section.id = sectionId;
    section.bigEndian = bigEndian;
    return sections.emplace_back(section);
}

bool Rx3Container::IsEmpty() {
    return sections.empty();
}
