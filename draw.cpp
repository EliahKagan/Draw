// Very limited turtle graphics program.

#include <algorithm>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
    using namespace std::literals;

    class Canvas {
    public:
        enum class Pen : bool { up, down };

        explicit Canvas(std::size_t width = 70u, char bg = ' ', char fg = '*',
                        char cur = 'X', Pen pen = Pen::up);

        void draw() const;

        // Instructions:       Names:
        void mark();        // m
        void clean();       // c
        void up();          // u
        void down();        // d
        void north();       // n, 8
        void south();       // s, 2
        void east();        // e, 6
        void west();        // w, 4
        void northeast();   // o, 9
        void northwest();   // i, 7
        void southeast();   // l, 3
        void southwest();   // k, 1

    private:
        [[nodiscard]] const bool& cell(std::size_t x, std::size_t y) const;
        [[nodiscard]] bool& cell(std::size_t x, std::size_t y);

        [[nodiscard, maybe_unused]] const bool& here() const;
        [[nodiscard]] bool& here();

        [[nodiscard]] char peek(std::size_t x, std::size_t y) const;

        void move_north();
        void move_south();
        void move_east();
        void move_west();

        void update(); // Currently, this just marks (if the pen is down).

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

    void Canvas::draw() const
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

    void Canvas::north()
    {
        move_north();
        update();
    }

    void Canvas::south()
    {
        move_south();
        update();
    }

    void Canvas::east()
    {
        move_east();
        update();
    }

    void Canvas::west()
    {
        move_west();
        update();
    }

    void Canvas::northeast()
    {
        move_north();
        move_east();
        update();
    }

    void Canvas::northwest()
    {
        move_north();
        move_west();
        update();
    }

    void Canvas::southeast()
    {
        move_south();
        move_east();
        update();
    }

    void Canvas::southwest()
    {
        move_south();
        move_west();
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

    void Canvas::move_north() // TODO: remove empty bottom row when cursor leaves it
    {
        if (y_ == 0u)
            rows_.emplace_front(width_);
        else
            --y_;
    }

    void Canvas::move_south() // TODO: remove empty top row when cursor leaves it
    {
        if (++y_ == size(rows_)) rows_.emplace_back(width_);
    }

    void Canvas::move_east()
    {
        if (x_ != width_ - 1u) {
            ++x_;
            return;
        }

        for (auto& row : rows_) {
            row.pop_front();
            row.emplace_back();
        }
    }

    void Canvas::move_west()
    {
        if (x_ != 0u) {
            --x_;
            return;
        }

        for (auto& row : rows_) {
            row.pop_back();
            row.emplace_front();
        }
    }

    inline void Canvas::update()
    {
        if (pen_ == Pen::down) mark();
    }

    class TranslationError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class ParsingError : public TranslationError {
    public:
        ParsingError();
    };

    ParsingError::ParsingError() : TranslationError{"Parsing error"}
    {
    }

    class AssemblyError : public TranslationError {
    public:
        explicit AssemblyError(char bad_instruction);
    };

    AssemblyError::AssemblyError(const char bad_instruction)
        : TranslationError{"Assembly error: unrecognized instruction: \""s
                            + bad_instruction + "\""}
    {
    }

    using Opcode = void (Canvas::*)();

    struct Instruction {
        std::string doc;
        std::string chars;
        Opcode opcode;
    };

    class Assembler {
    public:
        // Constructs an assembler for a user-specified instruction set.
        Assembler(std::initializer_list<Instruction> init);

        // Constructs an assembler with the default instruction set.
        Assembler();

        // Displays the documentation for each instruction.
        void help() const;

        // Reads "assembly language" from an input stream and assembles it.
        [[nodiscard]]
        std::vector<Opcode> operator()(std::istringstream& in) const;

    private:
        std::vector<Instruction> instruction_set_;
    };

    Assembler::Assembler(const std::initializer_list<Instruction> init)
        : instruction_set_(init) { }

    Assembler::Assembler() : Assembler{
        {"Mark the canvas here",    "m",    &Canvas::mark},
        {"Clean any mark here",     "c",    &Canvas::clean},
        {"take the pen Up",         "u",    &Canvas::up},
        {"put the pen Down",        "d",    &Canvas::down},
        {"move North",              "n8",   &Canvas::north},
        {"move South",              "s2",   &Canvas::south},
        {"move East",               "e6",   &Canvas::east},
        {"move West",               "w4",   &Canvas::west},
        {"move northeast",          "o9",   &Canvas::northeast},
        {"move northwest",          "i7",   &Canvas::northwest},
        {"move southeast",          "l3",   &Canvas::southeast},
        {"move southwest",          "k1",   &Canvas::southwest}
    } { };

    void Assembler::help() const
    {
        constexpr auto margin = "    ";
        const auto doc_heading = "DESCRIPTION"sv;

        auto doc_width = size(doc_heading);
        for (const auto& instr : instruction_set_)
            doc_width = std::max(doc_width, size(instr.doc));

        std::cout << margin << std::left << std::setw(doc_width) << doc_heading
                  << margin << "symbol(s)\n\n";

        for (const auto& instr : instruction_set_) {
            std::cout << margin << std::setw(doc_width) << instr.doc << margin;

            auto sep = "";
            for (const auto ch : instr.chars) {
                std::cout << sep << ch;
                sep = ", ";
            }

            std::cout << '\n';
        }
    }

    std::vector<Opcode> Assembler::operator()(std::istringstream& in) const
    {
        std::vector<Opcode> ret;

        const auto first = cbegin(instruction_set_);
        const auto last = cend(instruction_set_);

        for (char ch {}; in >> ch; ) {
            const auto p = std::find_if(first, last,
                                        [ch](const Instruction& instr) {
                return instr.chars.find(ch) != instr.chars.npos;
            });

            if (p == last) throw AssemblyError{ch};

            ret.push_back(p->opcode);
        }

        return ret;
    }

    unsigned extract_reps(std::istringstream& in)
    {
        if (in.get() != '\\') {
            in.unget();
            return 1;
        }

        unsigned ret {};
        if (!(in >> ret)) throw ParsingError{};
        return ret;
    }
}

int main()
{
    std::ios_base::sync_with_stdio(false);

    Canvas canvas;
    canvas.draw();

    for (; ; ) {
        std::cout << "\n? ";
        std::string script;
        if (!std::getline(std::cin, script)) break;

        try {
            std::istringstream in {script};
            auto reps = extract_reps(in);
            const auto code = assemble(in);

            while (reps-- != 0u)
                for (const auto f : code) (canvas.*f)();

            canvas.draw();
        }
        catch (const TranslationError& e) {
            std::cerr << e.what() << '\n';
        }
    }
}
