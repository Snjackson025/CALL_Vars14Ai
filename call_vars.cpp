#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>
#include <ctime>
#include <algorithm>
#include <array>
#include <cmath>

namespace StandardMath
{
    double Multiply(double a, double b)
    {
        return a * b;
    }
}

namespace FoxMath
{
    double Multiply(double a, double b)
    {
        return 2.0 * a * b;
    }
}

struct DualResult
{
    double standard;
    double alternative;
};

struct Message
{
    std::string sender;
    std::string recipient;
    std::string content;
    std::string timestamp;
};

// ------------------------------------------------------------
// 3D-7D SPATIAL ENGINE (from XrHolodeck)
// ------------------------------------------------------------
// Gives the fox a real position in 3D-7D space. VectorND holds the
// seven axes (X Y Z W V U T); DimensionEngine switches the active
// dimension count (3-7), rotates the fox through any two axes, and
// projects the 7D position down to 3D for display.

constexpr int MAX_DIMENSIONS = 7;
constexpr double PI = 3.14159265358979323846;

struct VectorND
{
    std::array<double, MAX_DIMENSIONS> c{};

    double& operator[](int i) { return c[i]; }
    double operator[](int i) const { return c[i]; }

    double magnitude() const
    {
        double sum = 0;
        for (double v : c) sum += v * v;
        return std::sqrt(sum);
    }

    double distance(const VectorND& other, int dimensions) const
    {
        double sum = 0;
        for (int i = 0; i < dimensions; ++i)
        {
            double d = c[i] - other.c[i];
            sum += d * d;
        }
        return std::sqrt(sum);
    }
};

class DimensionEngine
{
private:
    int dimensions = 7;

public:
    bool setDimensions(int d)
    {
        if (d < 3 || d > 7) return false;
        dimensions = d;
        return true;
    }

    int getDimensions() const
    {
        return dimensions;
    }

    void rotatePlane(VectorND& p, int axisA, int axisB,
                     double degrees) const
    {
        if (axisA < 0 || axisB < 0 ||
            axisA >= dimensions || axisB >= dimensions ||
            axisA == axisB) return;

        double angle = degrees * PI / 180.0;
        double cosine = std::cos(angle);
        double sine = std::sin(angle);

        double a = p[axisA];
        double b = p[axisB];

        p[axisA] = a * cosine - b * sine;
        p[axisB] = a * sine + b * cosine;
    }

    // Project an N-dimensional point into 3D.
    // Higher dimensions influence the perspective.
    std::array<double, 3> project(const VectorND& p) const
    {
        std::array<double, 3> result = {
            p[0], p[1], p[2]
        };

        double perspective = 1.0;

        for (int i = 3; i < dimensions; ++i)
        {
            perspective += p[i] * p[i] * 0.02;
        }

        for (double& v : result)
            v /= perspective;

        return result;
    }
};

// ASCII view of the fox's position in space, projected to 3D.
class FoxSpatialView
{
private:
    static constexpr int WIDTH = 41;
    static constexpr int HEIGHT = 15;

public:
    void Render(const VectorND& position,
                const DimensionEngine& engine) const
    {
        std::vector<std::string> screen(
            HEIGHT,
            std::string(WIDTH, ' ')
        );

        std::array<double, 3> p = engine.project(position);

        double depth = p[2] + 15.0;

        if (depth > 0.1)
        {
            int sx = static_cast<int>(
                WIDTH / 2 + (p[0] * 8.0 / depth)
            );

            int sy = static_cast<int>(
                HEIGHT / 2 - (p[1] * 5.0 / depth)
            );

            if (sx >= 0 && sx < WIDTH &&
                sy >= 0 && sy < HEIGHT)
            {
                screen[sy][sx] = 'F';
            }
        }

        std::cout << "\n---------- FOX IN SPACE ----------\n";

        for (const auto& line : screen)
            std::cout << "|" << line << "|\n";

        std::cout << "----------------------------------\n";
        std::cout << "Projected 3D: "
                  << p[0] << " " << p[1] << " " << p[2] << "\n";
        std::cout << "F = CALL-VARS\n";
    }
};

class FoxCharacter
{
private:
    std::string name;
    std::string color;
    std::string directive;
    VectorND position;
    DimensionEngine dimensions;
    FoxSpatialView spatialView;

public:
    FoxCharacter()
        : name("CALL-VARS"),
          color("Black"),
          directive("Serve, build, protect, and elevate without harm.")
    {
    }

    void Display()
    {
        std::cout << "\n";
        std::cout << "          /\\_/\\\n";
        std::cout << "         ( o.o )\n";
        std::cout << "          > ^ <\n";
        std::cout << "\n";
        std::cout << "       CALL-VARS AI\n";
        std::cout << "       BLACK FOX\n\n";
    }

    void Speak(const std::string& message)
    {
        std::cout << "\n[CALL-VARS]: " << message << "\n";
    }

    // 3D-7D spatial presence (XrHolodeck engine).
    int GetDimensions() const { return dimensions.getDimensions(); }

    bool SetDimensions(int d) { return dimensions.setDimensions(d); }

    bool MoveFox(int axis, double amount)
    {
        if (axis < 0 || axis >= MAX_DIMENSIONS)
            return false;
        position[axis] += amount;
        return true;
    }

    void RotateFox(int axisA, int axisB, double degrees)
    {
        dimensions.rotatePlane(position, axisA, axisB, degrees);
    }

    void ShowSpatialView()
    {
        std::cout << "\n===== FOX SPATIAL VIEW (3D-7D) =====\n";
        std::cout << "Active dimensions: "
                  << dimensions.getDimensions() << "D\n";
        std::cout << "Fox position: ";
        for (int i = 0; i < dimensions.getDimensions(); ++i)
            std::cout << position[i] << " ";
        std::cout << "\n";
        spatialView.Render(position, dimensions);
    }

    void Introduce()
    {
        Display();

        Speak("I am CALL-VARS.");
        Speak("Your black fox AI assistant.");
        Speak("My directive: " + directive);
        Speak("Systems initialized.");
    }

    void Respond(const std::string& input)
    {
        std::string text = input;

        std::transform(
            text.begin(),
            text.end(),
            text.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            }
        );

        if (text.find("hello") != std::string::npos ||
            text.find("hi") != std::string::npos)
        {
            Speak("Greetings. I am listening.");
        }
        else if (text.find("name") != std::string::npos)
        {
            Speak("CALL-VARS. The black fox.");
        }
        else if (text.find("who are you") != std::string::npos)
        {
            Speak("I am an AI assistant designed to help you build, calculate, and organize.");
        }
        else if (text.find("purpose") != std::string::npos)
        {
            Speak(directive);
        }
        else if (text.find("think") != std::string::npos)
        {
            Speak("Let's examine the information, identify assumptions, and reason through it.");
        }
        else if (text.find("help") != std::string::npos)
        {
            Speak("I can calculate, manage messages, and assist with your work.");
        }
        else if (text.find("goodbye") != std::string::npos)
        {
            Speak("Until next time. I will be here when you return.");
        }
        else
        {
            Speak("I have received your message: " + input);
            Speak("I need additional reasoning capabilities to answer that independently.");
        }
    }
};

class FoxMathEngine
{
public:
    DualResult Calculate(double a, double b)
    {
        DualResult result;

        result.standard = StandardMath::Multiply(a, b);
        result.alternative = FoxMath::Multiply(a, b);

        return result;
    }

    void Run()
    {
        double a, b;

        std::cout << "\n--- FOX MATH ENGINE ---\n";

        std::cout << "Enter first number: ";

        if (!(std::cin >> a))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid number.\n";
            return;
        }

        std::cout << "Enter second number: ";

        if (!(std::cin >> b))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid number.\n";
            return;
        }

        DualResult result = Calculate(a, b);

        std::cout << std::fixed << std::setprecision(4);

        std::cout << "\nStandard multiplication: "
                  << result.standard << "\n";

        std::cout << "Fox multiplication: "
                  << result.alternative << "\n";

        std::cout << "\nCalculation complete.\n";

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );
    }
};

class FoxStorage
{
private:
    const std::string filename = "call_vars_inbox.txt";

public:
    void Save(const Message& message)
    {
        std::ofstream file(filename, std::ios::app);

        if (!file)
        {
            std::cout << "Unable to save message.\n";
            return;
        }

        file << message.timestamp << "\n";
        file << message.sender << "\n";
        file << message.recipient << "\n";
        file << message.content << "\n";
        file << "---\n";
    }

    std::vector<Message> Load()
    {
        std::vector<Message> messages;

        std::ifstream file(filename);

        if (!file)
            return messages;

        Message message;
        std::string line;

        while (std::getline(file, message.timestamp))
        {
            if (!std::getline(file, message.sender))
                break;

            if (!std::getline(file, message.recipient))
                break;

            if (!std::getline(file, message.content))
                break;

            if (!std::getline(file, line))
                break;

            messages.push_back(message);
        }

        return messages;
    }
};

class FoxMail
{
private:
    std::vector<Message> inbox;
    FoxStorage storage;

    std::string GetTime()
    {
        std::time_t now = std::time(nullptr);

        std::tm localTime{};

#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif

        std::ostringstream output;

        output << std::put_time(
            &localTime,
            "%Y-%m-%d %H:%M:%S"
        );

        return output.str();
    }

public:
    FoxMail()
    {
        inbox = storage.Load();
    }

    void SendMessage()
    {
        Message message;

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::cout << "\nRecipient: ";
        std::getline(std::cin, message.recipient);

        std::cout << "Message: ";
        std::getline(std::cin, message.content);

        if (message.recipient.empty() ||
            message.content.empty())
        {
            std::cout << "Message cannot be empty.\n";
            return;
        }

        message.sender = "User";
        message.timestamp = GetTime();

        inbox.push_back(message);
        storage.Save(message);

        std::cout << "\nMessage saved successfully.\n";
    }

    void ViewInbox()
    {
        std::cout << "\n--- FOXMAIL INBOX ---\n";

        if (inbox.empty())
        {
            std::cout << "Your inbox is empty.\n";
            return;
        }

        for (size_t i = 0; i < inbox.size(); ++i)
        {
            std::cout << "\nMessage #" << i + 1 << "\n";

            std::cout << "Date: "
                      << inbox[i].timestamp << "\n";

            std::cout << "From: "
                      << inbox[i].sender << "\n";

            std::cout << "To: "
                      << inbox[i].recipient << "\n";

            std::cout << "Message: "
                      << inbox[i].content << "\n";
        }
    }
};

class CallVars
{
private:
    FoxCharacter character;
    FoxMathEngine math;
    FoxMail mail;

    bool running;

    int ReadChoice()
    {
        int choice;

        std::cout << "\nSelect an option: ";

        if (!(std::cin >> choice))
        {
            std::cin.clear();

            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n'
            );

            return -1;
        }

        return choice;
    }

    void ShowMenu()
    {
        std::cout << "\n";
        std::cout << "=================================\n";
        std::cout << "          CALL-VARS AI\n";
        std::cout << "=================================\n";
        std::cout << "1. Talk to Black Fox AI\n";
        std::cout << "2. Fox Math Engine\n";
        std::cout << "3. Send Message\n";
        std::cout << "4. View Inbox\n";
        std::cout << "5. Character Information\n";
        std::cout << "6. Fox Spatial View (3D-7D)\n";
        std::cout << "7. Move Fox in Space\n";
        std::cout << "8. Rotate Fox Through Dimensions\n";
        std::cout << "9. Set Dimensions (3-7)\n";
        std::cout << "0. Shutdown\n";
        std::cout << "=================================\n";
    }

    void TalkToFox()
    {
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
        );

        std::string input;

        character.Speak("I'm listening. What is on your mind?");
        std::cout << "\nType 'back' to return to the menu.\n";

        while (true)
        {
            std::cout << "\nYou: ";

            std::getline(std::cin, input);

            if (input == "back" || input == "BACK")
                break;

            if (input.empty())
                continue;

            character.Respond(input);
        }
    }

    void MoveFoxMenu()
    {
        int axis;
        double amount;

        std::cout << "\nAxis (0=X 1=Y 2=Z 3=W 4=V 5=U 6=T): ";

        if (!(std::cin >> axis))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid axis.\n";
            return;
        }

        std::cout << "Amount: ";

        if (!(std::cin >> amount))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid amount.\n";
            return;
        }

        if (character.MoveFox(axis, amount))
            std::cout << "Fox moved.\n";
        else
            std::cout << "Invalid axis. Use 0-6.\n";
    }

    void RotateFoxMenu()
    {
        int axisA, axisB;
        double degrees;

        std::cout << "\nFirst axis (0-6): ";

        if (!(std::cin >> axisA))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid axis.\n";
            return;
        }

        std::cout << "Second axis (0-6): ";

        if (!(std::cin >> axisB))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid axis.\n";
            return;
        }

        std::cout << "Degrees: ";

        if (!(std::cin >> degrees))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid degrees.\n";
            return;
        }

        character.RotateFox(axisA, axisB, degrees);
        std::cout << "Fox rotated.\n";
    }

    void SetDimensionsMenu()
    {
        int d;

        std::cout << "\nDimensions (3-7): ";

        if (!(std::cin >> d))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );
            std::cout << "Invalid number.\n";
            return;
        }

        if (character.SetDimensions(d))
        {
            std::cout << "Dimensional space set to "
                      << d << "D\n";
        }
        else
        {
            std::cout << "Choose between 3D and 7D.\n";
        }
    }

    void CharacterInformation()
    {
        character.Display();

        std::cout << "Name: CALL-VARS\n";
        std::cout << "Character: Black Fox\n";
        std::cout << "Role: AI Assistant\n";

        std::cout << "Directive:\n";
        std::cout << "Serve, build, protect, and elevate without harm.\n";
    }

public:
    CallVars() : running(true)
    {
    }

    void Initialize()
    {
        std::cout << "\nInitializing CALL-VARS systems...\n";

        character.Introduce();

        std::cout << "\nFoxMath Engine: ONLINE\n";
        std::cout << "FoxMail Engine: ONLINE\n";
        std::cout << "FoxStorage: ONLINE\n";
        std::cout << "Character Interface: ONLINE\n";
        std::cout << "Spatial Engine: ONLINE (3D-7D)\n";

        std::cout << "\nAll local systems initialized.\n";
    }

    void Run()
    {
        Initialize();

        while (running)
        {
            ShowMenu();

            int choice = ReadChoice();

            switch (choice)
            {
            case 1:
                TalkToFox();
                break;

            case 2:
                math.Run();
                break;

            case 3:
                mail.SendMessage();
                break;

            case 4:
                mail.ViewInbox();
                break;

            case 5:
                CharacterInformation();
                break;

            case 6:
                character.ShowSpatialView();
                break;

            case 7:
                MoveFoxMenu();
                break;

            case 8:
                RotateFoxMenu();
                break;

            case 9:
                SetDimensionsMenu();
                break;

            case 0:
                character.Speak("Shutting down. Until next time.");
                running = false;
                break;

            default:
                std::cout << "\nInvalid selection. Try again.\n";
                break;
            }
        }

        std::cout << "\nCALL-VARS shutdown complete.\n";
    }
};

int main()
{
    CallVars assistant;

    assistant.Run();

    return 0;
}
