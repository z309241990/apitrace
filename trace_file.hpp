/**************************************************************************
 *
 * Copyright 2011 Zack Rusin
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#ifndef TRACE_FILE_HPP
#define TRACE_FILE_HPP

#include <string>
#include <fstream>
#include <stdint.h>

namespace Trace {

class File {
public:
    enum Mode {
        Read,
        Write
    };
    struct Offset {
        Offset()
            : chunk(0),
              offsetInChunk(0)
        {}
        uint64_t chunk;
        uint32_t offsetInChunk;
    };

public:
    static bool isZLibCompressed(const std::string &filename);
    static bool isSnappyCompressed(const std::string &filename);
public:
    File(const std::string &filename = std::string(),
         File::Mode mode = File::Read);
    virtual ~File();

    bool isOpened() const;
    File::Mode mode() const;

    std::string filename() const;

    bool open(const std::string &filename, File::Mode mode);
    bool write(const void *buffer, size_t length);
    bool read(void *buffer, size_t length);
    void close();
    void flush(void);
    int getc();
    bool skip(unsigned length);

    virtual bool supportsOffsets() const = 0;
    virtual File::Offset currentOffset();
    virtual void setCurrentOffset(const File::Offset &offset);
protected:
    virtual bool rawOpen(const std::string &filename, File::Mode mode) = 0;
    virtual bool rawWrite(const void *buffer, size_t length) = 0;
    virtual bool rawRead(void *buffer, size_t length) = 0;
    virtual int rawGetc() = 0;
    virtual void rawClose() = 0;
    virtual void rawFlush() = 0;
    virtual bool rawSkip(unsigned length) = 0;

protected:
    std::string m_filename;
    File::Mode m_mode;
    bool m_isOpened;
};

inline bool File::isOpened() const
{
    return m_isOpened;
}

inline File::Mode File::mode() const
{
    return m_mode;
}

inline std::string File::filename() const
{
    return m_filename;
}

inline bool File::open(const std::string &filename, File::Mode mode)
{
    if (m_isOpened) {
        close();
    }
    m_isOpened = rawOpen(filename, mode);
    m_mode = mode;

    return m_isOpened;
}

inline bool File::write(const void *buffer, size_t length)
{
    if (!m_isOpened || m_mode != File::Write) {
        return false;
    }
    return rawWrite(buffer, length);
}

inline bool File::read(void *buffer, size_t length)
{
    if (!m_isOpened || m_mode != File::Read) {
        return false;
    }
    return rawRead(buffer, length);
}

inline void File::close()
{
    if (m_isOpened) {
        rawClose();
        m_isOpened = false;
    }
}

inline void File::flush(void)
{
    if (m_mode == File::Write) {
        rawFlush();
    }
}

inline int File::getc()
{
    if (!m_isOpened || m_mode != File::Read) {
        return -1;
    }
    return rawGetc();
}

inline bool File::skip(unsigned length)
{
    if (!m_isOpened || m_mode != File::Read) {
        return false;
    }
    return rawSkip(length);
}

class ZLibFile : public File {
public:
    ZLibFile(const std::string &filename = std::string(),
             File::Mode mode = File::Read);
    virtual ~ZLibFile();


    virtual bool supportsOffsets() const;
protected:
    virtual bool rawOpen(const std::string &filename, File::Mode mode);
    virtual bool rawWrite(const void *buffer, size_t length);
    virtual bool rawRead(void *buffer, size_t length);
    virtual int rawGetc();
    virtual void rawClose();
    virtual void rawFlush();
    virtual bool rawSkip(unsigned length);
private:
    void *m_gzFile;
};

inline bool
operator<(const File::Offset &one, const File::Offset &two)
{
    return one.chunk < two.chunk ||
            (one.chunk == two.chunk && one.offsetInChunk < two.offsetInChunk);
}

inline bool
operator==(const File::Offset &one, const File::Offset &two)
{
    return one.chunk == two.chunk &&
            one.offsetInChunk == two.offsetInChunk;
}

inline bool
operator>=(const File::Offset &one, const File::Offset &two)
{
    return one.chunk > two.chunk ||
            (one.chunk == two.chunk && one.offsetInChunk >= two.offsetInChunk);
}

inline bool
operator>(const File::Offset &one, const File::Offset &two)
{
    return two < one;
}

inline bool
operator<=(const File::Offset &one, const File::Offset &two)
{
    return two >= one;
}


}

#endif
