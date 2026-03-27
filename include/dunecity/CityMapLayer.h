#ifndef DUNECITY_CITYMAPLAYER_H
#define DUNECITY_CITYMAPLAYER_H

#include <cstdint>
#include <vector>

namespace DuneCity {

template<typename T>
class CityMapLayer {
public:
    CityMapLayer() = default;

    void init(int width, int height, int blockSize = 2) {
        blockSize_ = blockSize;
        width_  = (width  + blockSize - 1) / blockSize;
        height_ = (height + blockSize - 1) / blockSize;
        data_.assign(width_ * height_, T{});
    }

    int getBlockSize() const { return blockSize_; }

    T get(int x, int y) const {
        if(x < 0 || x >= width_ || y < 0 || y >= height_) return T{};
        return data_[y * width_ + x];
    }

    void set(int x, int y, T val) {
        if(x >= 0 && x < width_ && y >= 0 && y < height_)
            data_[y * width_ + x] = val;
    }

    T worldGet(int wx, int wy) const {
        return get(wx / blockSize_, wy / blockSize_);
    }

private:
    int blockSize_ = 2;
    int width_  = 0;
    int height_ = 0;
    std::vector<T> data_;
};

} // namespace DuneCity

#endif // DUNECITY_CITYMAPLAYER_H
