#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    class Canvas {
    public:
        enum class Pen : unsigned char { up, down };

        explicit Canvas(std::size_t width = 70, char bg = ' ', char fg = '*',
                        char cur = 'X', Pen pen = Pen::up);
        
        void draw() const;

    private:
        const bool& cell(std::size_t x, std::size_t y) const;
        bool& cell(std::size_t x, std::size_t y);

        char peek(std::size_t x, std::size_t y) const;

        std::deque<std::deque<bool>> rows_;
        size_t width_;
        size_t x_;
        size_t y_;
        char bg_;
        char fg_;
        char cur_;
        Pen pen_;
    };

    Canvas::Canvas(const std::size_t width, const char bg, const char fg,
                   const char cur, const Pen pen)
        : rows_{std::deque<bool>(width)}, width_{width}, x_{width / 2}, y_{0},
          bg_{bg}, fg_{fg}, cur_{cur}, pen_{pen}
    {
        if (width == 0) throw std::length_error{"zero-width canvas vanishes"};
        //rows_.at(y_).at(x_) = cur_;
    }

    void Canvas::draw() const // FIXME: rewrite to accommodate representation change
    {
        for (std::size_t y {0}; y != rows_.size(); ++y) {
            for (std::size_t x {0}; x != width_; ++x)
                std::cout.put(peek(x, y));
            
            std::cout.put('\n');
        }
    }

    inline const bool& Canvas::cell(const std::size_t x,
                                    const std::size_t y) const
    {
        return rows_.at(y).at(x);
    }

    inline bool& Canvas::cell(const std::size_t x, const std::size_t y)
    {
        return rows_.at(y).at(x);
    }

    inline char Canvas::peek(const std::size_t x, const std::size_t y) const
    {
        if (y == y_ && x == x_) return cur_;

        return cell(x, y) ? fg_ : bg_;
    }
}

int main()
{
    std::ios_base::sync_with_stdio(false);

    Canvas canvas;

    for (; ; ) {
        std::cout << '\n';
        canvas.draw();

        std::cout << "\n? ";
        std::string script;
        if (!std::getline(std::cin, script)) break;
        
        // FIXME: run the script!
    }
}
