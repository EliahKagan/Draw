#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    class Canvas {
    public:
        explicit Canvas(std::size_t width = 70,
                        char bg = ' ', char fg = '*', char cur = 'X');
        
        void draw() const;

    private:
        std::deque<std::deque<char>> rows_;
        size_t width_;
        size_t x_;
        size_t y_;
        char bg_;
        char fg_;
        char cur_;
    };

    Canvas::Canvas(const std::size_t width,
                   const char bg, const char fg, const char cur)
        : rows_{std::deque<char>(width, bg)}, width_{width},
          x_{width / 2}, y_{0}, bg_{bg}, fg_{fg}, cur_{cur}
    {
        if (width == 0) throw std::length_error{"zero-width canvas vanishes"};
        rows_.at(y_).at(x_) = cur_;
    }

    void Canvas::draw() const
    {
        for (const auto& row : rows_) {
            for (const auto ch : row) std::cout.put(ch);
            std::cout.put('\n');
        }
    }
}

int main()
{
    std::ios_base::sync_with_stdio(false);

    Canvas canvas;

    for (; ; ) {
        std::cout << '\n';
        canvas.draw();
        std::cout << "\n? " << std::flush;

        std::string script;
        std::cin >> script;
        
        // FIXME: run the script!
    }
}
