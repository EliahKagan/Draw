// Very limited turtle-like graphics program. (The turtle has no orientation.)

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace {
    using namespace std::literals;

    // Collects lambdas (or other functors) to use as overloads for a new
    // functor's function call operator. Useful for std::visit.
    // See "overloaded" in http://stroustrup.com/tour2.html, p. 176.
    template<typename... Fs>
    class MultiLambda : public Fs... {
    public:
        using Fs::operator()...;
    };

    // Use each functor as a base-class subobject. Their signatures must differ.
    template<typename... Fs>
    MultiLambda(Fs...) -> MultiLambda<Fs...>;

    // A text-based canvas that expands vertically and truncates horizontally.
    class Canvas {
    public:
        // Pen state. The pen marks automatically when it is put down and when
        // it is moved in any direction while down.
        enum class Pen : bool { up, down };

        // Constructs a canvas with the specified width (in columns), background
        // symbol, foreground symbol, current position / cursor sumbol, and pen
        // state (up or down).
        explicit Canvas(std::size_t width = 70u, char bg = ' ', char fg = '*',
                        char cur = 'X', Pen pen = Pen::up);

        // INSTRUCTIONS:                                           NAMES:

        // Makes a dot at the current position.
        void mark() noexcept;                                   // m

        // Erases a dot at the current position.
        void clean() noexcept;                                  // c

        // Takes the pen up (i.e., stops auto-marking).
        void up() noexcept;                                     // u

        // Puts the pen down (i.e, starts auto-marking).
        void down() noexcept;                                   // d

        // Moves the pen north (upward on the screen).
        void north();                                           // n, 8

        // Moves the pen south (downward on the screen).
        void south();                                           // s, 2

        // Moves the pen east (right on the screen).
        void east();                                            // e, 6

        // Moves the pen west (left on the screen).
        void west();                                            // w, 4

        // Moves the pen northeast (up-right on the screen).
        void northeast();                                       // o, 9

        // Moves the pen northwest (up-left on the screen).
        void northwest();                                       // i, 7

        // Moves the pen southeast (down-right on the screen).
        void southeast();                                       // l, 3

        // Moves the pen southwest (down-left on the screen).
        void southwest();                                       // k, 1

        // Resize the canvas to remove the portion above here.
        //void crop_above();                                      // a

        // Resize the canvas to remove the portion below here.
        //void crop_below();                                      // b

        // ^^^ END OF INSTRUCTIONS ^^^

        friend std::ostream& operator<<(std::ostream& out,
                                        const Canvas& canvas);

    private:
        // These helpers use at() for bounds checking, to mitigate possible
        // bugs. But we don't catch std::out_of_range. Program termination, as
        // occurs when an exception would propogate out of a noexcept function,
        // is the least bad of all possible behaviors in such a situation.

        // The cell at the given coordinates (for reading).
        [[nodiscard]]
        const bool& cell(std::size_t x, std::size_t y) const noexcept;

        // The cell at the given coordinates (for reading or writing).
        [[nodiscard]] bool& cell(std::size_t x, std::size_t y) noexcept;

        // The cell at the current position (for reading).
        [[nodiscard, maybe_unused]] const bool& here() const noexcept;

        // The cell at the current position (for reading or writing).
        [[nodiscard]] bool& here() noexcept;

        // The symbolic representation for the cell at the given coordinates.
        [[nodiscard]] char peek(std::size_t x, std::size_t y) const noexcept;

        // Moves north, but does not call any updaters.
        void move_north();

        // Moves south, but does not call any updaters.
        void move_south();

        // Moves east, but does not call any updaters.
        void move_east();

        // Moves west, but does not call any updaters.
        void move_west();

        // Performs whatever actions should be done after each complete change
        // of cursor position. Currently, this just marks (if the pen is down).
        void update();

        // The grid holding the pattern recorded on the canvas, stored as rows.
        std::deque<std::deque<bool>> rows_;

        // The width of the canvas, in columns.
        size_t width_;

        // The column that the cursor currently resides in.
        size_t x_;

        // The row that the cursor currently resides in.
        size_t y_;

        // The symbolic representation for unmarked (background) cells.
        char bg_;

        // The symbolic representation for marked (foreground) cells.
        char fg_;

        // The symbolic representation for the cursor itself.
        char cur_;

        // The state the pen is currently in (whether it is up or down).
        Pen pen_;
    };

    Canvas::Canvas(const std::size_t width, const char bg, const char fg,
                   const char cur, const Pen pen)
        : rows_{std::deque<bool>(width)}, width_{width}, x_{width / 2u}, y_{0u},
          bg_{bg}, fg_{fg}, cur_{cur}, pen_{pen}
    {
        if (width == 0) throw std::length_error{"zero-width canvas vanishes"};
    }

    void Canvas::mark() noexcept
    {
        here() = true;
    }

    void Canvas::clean() noexcept
    {
        here() = false;
    }

    void Canvas::up() noexcept
    {
        pen_ = Pen::up;
    }

    void Canvas::down() noexcept
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

    // Draws the pattern of foreground dots that are recorded on the canvas.
    std::ostream& operator<<(std::ostream& out, const Canvas& canvas)
    {
        for (std::size_t y {0u}; y != size(canvas.rows_); ++y) {
            for (std::size_t x {0u}; x != canvas.width_; ++x)
                out.put(canvas.peek(x, y));

            out.put('\n');
        }

        return out;
    }

    inline const bool& Canvas::cell(const std::size_t x,
                                    const std::size_t y) const noexcept
    {
        return rows_.at(y).at(x);
    }

    inline bool& Canvas::cell(const std::size_t x, const std::size_t y) noexcept
    {
        return rows_.at(y).at(x);
    }

    inline const bool& Canvas::here() const noexcept
    {
        return cell(x_, y_);
    }

    inline bool& Canvas::here() noexcept
    {
        return cell(x_, y_);
    }

    inline char
    Canvas::peek(const std::size_t x, const std::size_t y) const noexcept
    {
        if (y == y_ && x == x_) return cur_;

        return cell(x, y) ? fg_ : bg_;
    }

    void Canvas::move_north()
    {
        if (y_ == 0u)
            rows_.emplace_front(width_);
        else
            --y_;
    }

    void Canvas::move_south()
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

    // Abstract base class for exceptions to throw when a user-provided script
    // contains an error that prevents it from being assembled or otherwise
    // used.
    class TranslationError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;

        // Just makes the class abstract. (No special cleanup needed.)
        ~TranslationError() override = 0;
    };

    TranslationError::~TranslationError() = default;

    // Throw this when we have no idea what sort of thing the user means for
    // their script to do.
    class ParsingError : public TranslationError {
    public:
        ParsingError();
    };

    // Constructs a ParsingError. There isn't any specific information to give
    // the user, so no arguments are accepted.
    ParsingError::ParsingError() : TranslationError{"Parsing error"}
    {
    }

    // Throw this when the user's script has the correct basic syntax but
    // contains an unrecognized instruction.
    class AssemblyError : public TranslationError {
    public:
        explicit AssemblyError(char bad_instruction);
    };

    // Constructs an AssemblyError from the unrecognized instruction. If there
    // were mutliple unrecognized instructions, just pass the first one.
    AssemblyError::AssemblyError(const char bad_instruction)
        : TranslationError{"Assembly error: unrecognized instruction: \""s
                            + bad_instruction + "\""}
    {
    }

    // Opcodes are pointers to the public member functions of Canvas. Those
    // functions comprise its interface. We provide an instruction to allow the
    // user to call each of them. (But not the Canvas constructor, of course.)
    using Opcode = void (Canvas::*)();

    // Information about an instruction that an Assembler must know.
    struct Instruction {
        // A brief human-readable summary of what the instruction does.
        // This is included when an Assembler is printed (to provide help).
        std::string doc;

        // The symbols that denote the instruction. We map them to its opcode.
        std::string chars;

        // Pointer to the public member function of Canvas. This is the target
        // "format" into which symbols for the instruction are translated.
        Opcode opcode;
    };

    // Translator of one-character symbols into callable "opcodes" (which are
    // pointers to member functions of Canvas). Also stores help information.
    class Assembler {
    public:
        // Constructs an assembler for a user-specified instruction set.
        Assembler(std::initializer_list<Instruction> init);

        // Constructs an assembler with the default instruction set.
        Assembler();

        // Reads "assembly language" from an input stream and assembles it.
        [[nodiscard]]
        std::vector<Opcode> operator()(std::istream& in) const;

        friend std::ostream& operator<<(std::ostream& out, const Assembler& as);

    private:
        // All the instructions the assembler accepts. Because we are using so
        // few instructions, it is reasonable (and probably even faster) to use
        // a vector for these, rather than some associative container.
        std::vector<Instruction> instruction_set_;
    };

    Assembler::Assembler(const std::initializer_list<Instruction> init)
        : instruction_set_(init)
    {
    }

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
        {"move southwest",          "k1",   &Canvas::southwest}}
    {
    }

    std::vector<Opcode> Assembler::operator()(std::istream& in) const
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

    // Returns the heading and (maximum) width of the column displaying
    // instructions' documentation strings. (Helper to print Assembler.)
    [[nodiscard]] std::tuple<std::string_view, int>
    doc_heading_and_width(const std::vector<Instruction>& instruction_set)
    {
        const auto doc_heading = "DESCRIPTION"sv;

        auto width = size(doc_heading);

        for (const auto& instruction : instruction_set)
            width = std::max(width, size(instruction.doc));

        return {doc_heading, static_cast<int>(width)};
    }

    // Lists one or more characters separately. (Helper to print Assembler.)
    void output_chars(std::ostream& out, const std::string_view chars)
    {
        auto sep = "";
        for (const auto ch : chars) {
            out << sep << ch;
            sep = ", ";
        }
    }

    // Displays the documentation for each instruction.
    std::ostream& operator<<(std::ostream& out, const Assembler& as)
    {
        constexpr auto margin = "    ";

        const auto [doc_heading, doc_width] =
                doc_heading_and_width(as.instruction_set_);

        out << margin << std::left << std::setw(doc_width) << doc_heading
            << margin << "SYMBOL(s)\n\n";

        for (const auto& instruction : as.instruction_set_) {
            out << margin << std::setw(doc_width) << instruction.doc << margin;
            output_chars(out, instruction.chars);
            out << '\n';
        }

        return out;
    }

    // Briefly tells the user how to get help and how to quit the program.
    void show_quick_help()
    {
        std::cerr << "Use \"?\" or \"\\h\" for help, and \"\\q\" to quit.\n";
    }

    // Tells the user how to do perform just about every supported action.
    void show_help(const Assembler& as)
    {
        std::cerr << as << '\n';
        std::cerr << "To repeat an instruction N times,"
                     " put \\N at the beginning of the line.\n";
        std::cerr << "If the next symbol is also a numeral,"
                     " type a space (or tab) before it.\n\n";
        show_quick_help();
    }

    // Prints a message and quits with a specified exit status.
    [[noreturn]] void quit(const int status, const std::string_view message)
    {
        std::cerr << message << '\n';
        std::exit(status);
    }

    // Prompts the user and reads a response, returning it as a stringstream to
    // faciliate parsing. Returns std::nullopt only when stdin is end-of-input.
    [[nodiscard]] std::optional<std::istringstream> read_script_as_stream()
    {
        std::cerr << "\n? ";
        std::string script;
        if (getline(std::cin, script)) return std::istringstream{script};
        return std::nullopt;
    }

    // Special actions a leading backslash may prefix (if not for repetition).
    namespace specials {
        // Designates that the full help message should be printed.
        constexpr struct HelpTag { } help;

        // Designates that the program should be quit.
        constexpr struct QuitTag { } quit;
    }

    // Extracts an integer from a stream and tries to use it as a rep-count.
    [[nodiscard]] int extract_reps(std::istream& in)
    {
        int reps {};
        if (!(in >> reps) || reps < 0) throw ParsingError{};
        return reps;
    }

    // Interprets leading-backslash notation, which the user may use to provide
    // a custom repetition count for the instructions int he rest of their
    // script, or to view the full help message or quit the program.
    [[nodiscard]] std::variant<int, specials::HelpTag, specials::QuitTag>
    extract_reps_or_special_action(std::istream& in)
    {
        in >> std::ws;

        switch (in.get()) {
        case '?':
            return specials::help;

        case '\\':
            switch (in.get()) {
                case 'h':
                case 'H':
                case '?':
                    return specials::help;

                case 'q':
                case 'Q':
                    return specials::quit;

                default:
                    in.unget();
                    return extract_reps(in);
            }

        default:
            in.unget();
            return 1;
        }
    }

    // Execute assembled opcodes on a canvas a specified number of times.
    void execute(Canvas& canvas, const std::vector<Opcode>& code, int reps)
    {
        while (reps-- != 0)
            for (const auto f : code) (canvas.*f)();

        std::cout << canvas;
    }

    // Main loop. Runs the user's commands. Displays the canvas except on error.
    void repl(const Assembler& as, Canvas& canvas)
    {
        while (auto in = read_script_as_stream()) {
            try {
                visit(MultiLambda{
                    [&](const int reps) { execute(canvas, as(*in), reps); },
                    [&](specials::HelpTag) { show_help(as); },
                    [](specials::QuitTag) { quit(EXIT_SUCCESS, "Bye!"); }
                }, extract_reps_or_special_action(*in));
            }
            catch (const TranslationError& e) {
                std::cerr << e.what() << '\n';
                show_quick_help();
            }
        }
    }
}

// Makes an assembler and canvas, displays initial output, and enters the REPL.
int main()
{
    std::ios_base::sync_with_stdio(false);

    try {
        const Assembler as;

        show_quick_help();
        std::cerr << '\n';

        Canvas canvas;
        std::cout << canvas;

        repl(as, canvas);
    }
    catch (const std::bad_alloc&) {
        std::cerr << "Out of memory!\n";
        return EXIT_FAILURE;
    }
}
