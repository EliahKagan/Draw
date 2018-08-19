#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
    class Canvas {
    public:
        enum class Pen : unsigned char { up, down };

        explicit Canvas(std::size_t width = 70u, char bg = ' ', char fg = '*',
                        char cur = 'X', Pen pen = Pen::up);
        
        void draw() const;

        // Instructions:       Names:
        void mark();        // m
        void clean();       // c
        void up();          // u
        void down();        // d
        void north();       // n
        void south();       // s
        void east();        // e
        void west();        // w

    private:
        const bool& cell(std::size_t x, std::size_t y) const;
        bool& cell(std::size_t x, std::size_t y);

        const bool& here() const;
        bool& here();

        char peek(std::size_t x, std::size_t y) const;

        void update(); // currently just marks if the pen is down

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
        : rows_{std::deque<bool>(width)}, width_{width}, x_{width / 2u}, y_{0u},
          bg_{bg}, fg_{fg}, cur_{cur}, pen_{pen}
    {
        if (width == 0) throw std::length_error{"zero-width canvas vanishes"};
    }

    void Canvas::draw() const // FIXME: rewrite to accommodate representation change
    {
        for (std::size_t y {0u}; y != size(rows_); ++y) {
            for (std::size_t x {0u}; x != width_; ++x)
                std::cout.put(peek(x, y));
            
            std::cout.put('\n');
        }
    }

    void Canvas::mark()
    {
        here() = true;
    }

    void Canvas::clean()
    {
        here() = false;
    }

    void Canvas::up()
    {
        pen_ = Pen::up;
    }

    void Canvas::down()
    {
        pen_ = Pen::down;
        mark();
    }

    void Canvas::north() // TODO: remove empty bottom row when cursor leaves it
    {
        if (y_ == 0u)
            rows_.emplace_front(width_);
        else
            --y_;
        
        update();
    }

    void Canvas::south() // TODO: remove empty top row when cursor leaves it
    {
        if (++y_ == size(rows_))
            rows_.emplace_back(width_);
        
        update();
    }

    void Canvas::east()
    {
        if (x_ == size(rows_) - 1u) {
            for (auto& row : rows_) {
                row.pop_front();
                row.emplace_back();
            }
        }
        else ++x_;

        update();
    }

    void Canvas::west()
    {
        if (x_ == 0u) {
            for (auto& row : rows_) {
                row.pop_back();
                row.emplace_front();
            }
        }
        else --x_;

        update();
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

    inline const bool& Canvas::here() const
    {
        return cell(x_, y_);
    }

    inline bool& Canvas::here()
    {
        return cell(x_, y_);
    }

    inline char Canvas::peek(const std::size_t x, const std::size_t y) const
    {
        if (y == y_ && x == x_) return cur_;

        return cell(x, y) ? fg_ : bg_;
    }

    inline void Canvas::update()
    {
        if (pen_ == Pen::down) mark();
    }

    using Opcode = void (Canvas::*)();

    std::vector<Opcode> assemble(const std::string& script)
    {
        static const std::unordered_map<char, Opcode> table {
            {'m', &Canvas::mark},
            {'c', &Canvas::clean},
            {'u', &Canvas::up},
            {'d', &Canvas::down},
            {'n', &Canvas::north},
            {'s', &Canvas::south},
            {'e', &Canvas::east},
            {'w', &Canvas::west}
        };

        std::vector<Opcode> ret;
        std::istringstream in {script};
        for (char ch {}; in >> ch; ) ret.push_back(table.at(ch));
        return ret;
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
        
        try {
            for (const auto f : assemble(script)) (canvas.*f)();
        }
        catch (const std::out_of_range& e) { // TODO: use custom exception type
            std::cerr << "Assembly error: unrecognized instruction\n";
        }
    }
}
