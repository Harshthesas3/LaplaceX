#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cwctype>

using namespace std;

#define ID_INPUT 101
#define ID_CALCULATE 102
#define ID_ANALYZE 103
#define ID_CLEAR 104
#define ID_SAVE 105
#define ID_LIBRARY 106
#define ID_ABOUT 107
#define ID_HISTORY 108
#define ID_CATEGORY 109
#define ID_RESULT 110
#define ID_FORMULA 111
#define ID_EXPLANATION 112
#define ID_STATUS 113

enum class FunctionType
{
    Constant,
    Polynomial,
    Exponential,
    Sine,
    Cosine,
    Sinh,
    Cosh,
    Invalid
};

struct ParsedFunction
{
    FunctionType type = FunctionType::Invalid;
    wstring original;
    double value = 0.0;
    int power = 0;
    wstring error;
};

struct HistoryEntry
{
    wstring functionText;
    wstring transform;
    wstring timestamp;
};

class TextUtil
{
public:
    static wstring Trim(const wstring& s)
    {
        size_t first = s.find_first_not_of(L" \t\r\n");
        if (first == wstring::npos) return L"";
        size_t last = s.find_last_not_of(L" \t\r\n");
        return s.substr(first, last - first + 1);
    }

    static wstring LowerNoSpaces(wstring s)
    {
        wstring out;
        for (wchar_t ch : s)
        {
            if (!iswspace(ch)) out.push_back((wchar_t)towlower(ch));
        }
        return out;
    }

    static bool IsNumber(const wstring& s, double& value)
    {
        if (s.empty()) return false;
        wchar_t* end = nullptr;
        value = wcstod(s.c_str(), &end);
        return end && *end == L'\0';
    }

    static wstring FormatNumber(double x)
    {
        if (abs(x - (long long)x) < 0.0000001)
        {
            return to_wstring((long long)x);
        }

        wstringstream ss;
        ss << fixed << setprecision(4) << x;
        wstring out = ss.str();
        while (!out.empty() && out.back() == L'0') out.pop_back();
        if (!out.empty() && out.back() == L'.') out.pop_back();
        return out;
    }

    static string Narrow(const wstring& ws)
    {
        if (ws.empty()) return "";
        int size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        string out(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &out[0], size, nullptr, nullptr);
        return out;
    }

    static wstring Now()
    {
        time_t now = time(nullptr);
        tm localTime{};
        localtime_s(&localTime, &now);

        wstringstream ss;
        ss << setfill(L'0')
            << setw(2) << localTime.tm_mday << L"-"
            << setw(2) << (localTime.tm_mon + 1) << L"-"
            << (localTime.tm_year + 1900) << L" "
            << setw(2) << localTime.tm_hour << L":"
            << setw(2) << localTime.tm_min << L":"
            << setw(2) << localTime.tm_sec;
        return ss.str();
    }
};

class FunctionParser
{
public:
    ParsedFunction Parse(const wstring& input)
    {
        ParsedFunction pf;
        pf.original = TextUtil::Trim(input);

        if (pf.original.empty())
        {
            pf.error = L"Input cannot be empty.";
            return pf;
        }

        wstring s = TextUtil::LowerNoSpaces(pf.original);
        double c = 0.0;

        if (TextUtil::IsNumber(s, c))
        {
            pf.type = FunctionType::Constant;
            pf.value = c;
            return pf;
        }

        if (s == L"t")
        {
            pf.type = FunctionType::Polynomial;
            pf.power = 1;
            return pf;
        }

        if (s.rfind(L"t^", 0) == 0)
        {
            wstring p = s.substr(2);
            if (!p.empty() && all_of(p.begin(), p.end(), iswdigit))
            {
                int n = stoi(p);
                if (n >= 0 && n <= 20)
                {
                    pf.type = FunctionType::Polynomial;
                    pf.power = n;
                    return pf;
                }
                pf.error = L"Polynomial power must be between 0 and 20.";
                return pf;
            }
        }

        if (ParseWrapped(s, L"exp(", L")", c))
        {
            pf.type = FunctionType::Exponential;
            pf.value = c;
            return pf;
        }

        if (ParseWrapped(s, L"sinh(", L")", c))
        {
            pf.type = FunctionType::Sinh;
            pf.value = c;
            return pf;
        }

        if (ParseWrapped(s, L"cosh(", L")", c))
        {
            pf.type = FunctionType::Cosh;
            pf.value = c;
            return pf;
        }

        if (ParseWrapped(s, L"sin(", L")", c))
        {
            pf.type = FunctionType::Sine;
            pf.value = c;
            return pf;
        }

        if (ParseWrapped(s, L"cos(", L")", c))
        {
            pf.type = FunctionType::Cosine;
            pf.value = c;
            return pf;
        }

        pf.error = L"Invalid syntax or unsupported function.";
        return pf;
    }

private:
    bool ParseWrapped(const wstring& s, const wstring& prefix, const wstring& suffix, double& a)
    {
        if (s.rfind(prefix, 0) != 0) return false;
        if (s.size() <= prefix.size() + suffix.size()) return false;
        if (s.substr(s.size() - suffix.size()) != suffix) return false;

        wstring inner = s.substr(prefix.size(), s.size() - prefix.size() - suffix.size());
        return ParseCoefficientTimesT(inner, a);
    }

    bool ParseCoefficientTimesT(wstring inner, double& a)
    {
        size_t star = inner.find(L'*');
        if (star != wstring::npos)
        {
            inner.erase(star, 1);
        }

        if (inner == L"t")
        {
            a = 1.0;
            return true;
        }

        if (inner == L"-t")
        {
            a = -1.0;
            return true;
        }

        if (inner.size() < 2 || inner.back() != L't') return false;

        wstring coef = inner.substr(0, inner.size() - 1);
        if (coef == L"+") coef = L"1";
        if (coef == L"-") coef = L"-1";

        return TextUtil::IsNumber(coef, a);
    }
};

class LaplaceEngine
{
public:
    wstring Category(const ParsedFunction& pf)
    {
        switch (pf.type)
        {
        case FunctionType::Constant: return L"Constant";
        case FunctionType::Polynomial: return L"Polynomial";
        case FunctionType::Exponential: return L"Exponential";
        case FunctionType::Sine: return L"Trigonometric - Sine";
        case FunctionType::Cosine: return L"Trigonometric - Cosine";
        case FunctionType::Sinh: return L"Hyperbolic - Sinh";
        case FunctionType::Cosh: return L"Hyperbolic - Cosh";
        default: return L"Invalid";
        }
    }

    wstring Transform(const ParsedFunction& pf)
    {
        wstring a = TextUtil::FormatNumber(pf.value);
        wstring a2 = TextUtil::FormatNumber(pf.value * pf.value);

        switch (pf.type)
        {
        case FunctionType::Constant:
            return TextUtil::FormatNumber(pf.value) + L"/s";
        case FunctionType::Polynomial:
            return FactorialString(pf.power) + L"/s^" + to_wstring(pf.power + 1);
        case FunctionType::Exponential:
            return L"1/(s - " + a + L")";
        case FunctionType::Sine:
            return a + L"/(s^2 + " + a2 + L")";
        case FunctionType::Cosine:
            return L"s/(s^2 + " + a2 + L")";
        case FunctionType::Sinh:
            return a + L"/(s^2 - " + a2 + L")";
        case FunctionType::Cosh:
            return L"s/(s^2 - " + a2 + L")";
        default:
            return L"";
        }
    }

    wstring Formula(const ParsedFunction& pf)
    {
        switch (pf.type)
        {
        case FunctionType::Constant:
            return L"L{c} = c/s";
        case FunctionType::Polynomial:
            return L"L{t^n} = n!/s^(n+1)";
        case FunctionType::Exponential:
            return L"L{e^(at)} = 1/(s-a)";
        case FunctionType::Sine:
            return L"L{sin(at)} = a/(s^2+a^2)";
        case FunctionType::Cosine:
            return L"L{cos(at)} = s/(s^2+a^2)";
        case FunctionType::Sinh:
            return L"L{sinh(at)} = a/(s^2-a^2)";
        case FunctionType::Cosh:
            return L"L{cosh(at)} = s/(s^2-a^2)";
        default:
            return L"";
        }
    }

    wstring Explanation(const ParsedFunction& pf)
    {
        wstring a = TextUtil::FormatNumber(pf.value);

        switch (pf.type)
        {
        case FunctionType::Constant:
            return L"The input is a constant c = " + a + L". Its Laplace transform is c divided by s.";
        case FunctionType::Polynomial:
            return L"The input is t raised to power n = " + to_wstring(pf.power) +
                L". The program computes n! = " + FactorialString(pf.power) +
                L" and places it over s^(n+1).";
        case FunctionType::Exponential:
            return L"The input matches exp(at) with a = " + a +
                L". The transform shifts s by a.";
        case FunctionType::Sine:
            return L"The input matches sin(at) with a = " + a +
                L". The numerator is a and the denominator is s^2+a^2.";
        case FunctionType::Cosine:
            return L"The input matches cos(at) with a = " + a +
                L". The numerator is s and the denominator is s^2+a^2.";
        case FunctionType::Sinh:
            return L"The input matches sinh(at) with a = " + a +
                L". The denominator uses s^2-a^2.";
        case FunctionType::Cosh:
            return L"The input matches cosh(at) with a = " + a +
                L". The numerator is s and the denominator uses s^2-a^2.";
        default:
            return L"Unable to analyze the input.";
        }
    }

private:
    unsigned long long Factorial(int n)
    {
        unsigned long long f = 1;
        for (int i = 2; i <= n; ++i) f *= i;
        return f;
    }

    wstring FactorialString(int n)
    {
        return to_wstring(Factorial(n));
    }
};

class HistoryManager
{
    vector<HistoryEntry> entries;

public:
    void Add(const wstring& functionText, const wstring& transform)
    {
        entries.push_back({ functionText, transform, TextUtil::Now() });
    }

    const vector<HistoryEntry>& Entries() const
    {
        return entries;
    }

    bool Save(const wstring& fileName)
    {
        ofstream file(TextUtil::Narrow(fileName), ios::out | ios::app);
        if (!file.is_open()) return false;

        file << "==== LaplaceX Ultimate History ====\n";
        for (const auto& e : entries)
        {
            file << "Time: " << TextUtil::Narrow(e.timestamp) << "\n";
            file << "Function: " << TextUtil::Narrow(e.functionText) << "\n";
            file << "Transform: " << TextUtil::Narrow(e.transform) << "\n";
            file << "-----------------------------------\n";
        }
        file.close();
        return true;
    }
};

class FormulaLibrary
{
public:
    static wstring Text()
    {
        return
            L"Laplace Transform Formula Library\n\n"
            L"Constants\n"
            L"  L{c} = c/s\n"
            L"  Examples: 1, 5, 10\n\n"
            L"Polynomials\n"
            L"  L{t^n} = n!/s^(n+1)\n"
            L"  Examples: t, t^2, t^3, t^10\n\n"
            L"Exponentials\n"
            L"  L{e^(at)} = 1/(s-a)\n"
            L"  Examples: exp(2t), exp(5t)\n\n"
            L"Sine\n"
            L"  L{sin(at)} = a/(s^2+a^2)\n"
            L"  Examples: sin(2t), sin(5t)\n\n"
            L"Cosine\n"
            L"  L{cos(at)} = s/(s^2+a^2)\n"
            L"  Examples: cos(2t), cos(5t)\n\n"
            L"Hyperbolic\n"
            L"  L{sinh(at)} = a/(s^2-a^2)\n"
            L"  L{cosh(at)} = s/(s^2-a^2)\n"
            L"  Examples: sinh(2t), cosh(3t)";
    }
};

class ThemeManager
{
public:
    HBRUSH backgroundBrush;
    HBRUSH panelBrush;
    HBRUSH inputBrush;
    COLORREF bg = RGB(22, 25, 31);
    COLORREF panel = RGB(32, 37, 46);
    COLORREF input = RGB(42, 48, 59);
    COLORREF text = RGB(236, 240, 245);
    COLORREF muted = RGB(176, 185, 196);
    COLORREF accent = RGB(74, 144, 226);

    ThemeManager()
    {
        backgroundBrush = CreateSolidBrush(bg);
        panelBrush = CreateSolidBrush(panel);
        inputBrush = CreateSolidBrush(input);
    }

    ~ThemeManager()
    {
        DeleteObject(backgroundBrush);
        DeleteObject(panelBrush);
        DeleteObject(inputBrush);
    }

    void ApplyFont(HWND hwnd, HFONT font)
    {
        SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    }
};

class MainWindow
{
    HWND hwnd = nullptr;
    HWND inputBox = nullptr;
    HWND categoryBox = nullptr;
    HWND resultBox = nullptr;
    HWND formulaBox = nullptr;
    HWND explanationBox = nullptr;
    HWND historyList = nullptr;
    HWND statusBar = nullptr;

    HFONT titleFont = nullptr;
    HFONT normalFont = nullptr;
    HFONT monoFont = nullptr;

    ThemeManager theme;
    FunctionParser parser;
    LaplaceEngine engine;
    HistoryManager history;

public:
    bool Create(HINSTANCE hInstance)
    {
        WNDCLASS wc{};
        wc.lpfnWndProc = MainWindow::WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"LaplaceXUltimateWindow";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = theme.backgroundBrush;

        RegisterClass(&wc);

        hwnd = CreateWindowEx(
            0,
            wc.lpszClassName,
            L"LaplaceX Ultimate",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1000,
            700,
            nullptr,
            nullptr,
            hInstance,
            this
        );

        return hwnd != nullptr;
    }

    void Show(int nCmdShow)
    {
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
    }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        MainWindow* self = nullptr;

        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            self = (MainWindow*)cs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
            self->hwnd = hwnd;
        }
        else
        {
            self = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (self)
        {
            return self->HandleMessage(msg, wParam, lParam);
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CREATE:
            BuildUi();
            return 0;

        case WM_COMMAND:
            HandleCommand(LOWORD(wParam));
            return 0;

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, theme.text);
            SetBkColor(hdc, theme.bg);
            return (LRESULT)theme.backgroundBrush;
        }

        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, theme.text);
            SetBkColor(hdc, theme.input);
            return (LRESULT)theme.inputBrush;
        }

        case WM_DESTROY:
            if (titleFont) DeleteObject(titleFont);
            if (normalFont) DeleteObject(normalFont);
            if (monoFont) DeleteObject(monoFont);
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    void BuildUi()
    {
        titleFont = CreateFont(34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

        normalFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

        monoFont = CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");

        AddLabel(L"LaplaceX Ultimate", 30, 20, 420, 45, titleFont);

        AddGroup(L"Function Input", 30, 85, 930, 145);
        AddLabel(L"Function Input:", 55, 120, 140, 25, normalFont);
        inputBox = AddEdit(L"", 200, 116, 430, 32, ID_INPUT);
        AddButton(L"Calculate", 655, 112, 120, 34, ID_CALCULATE);
        AddButton(L"Analyze", 790, 112, 120, 34, ID_ANALYZE);

        AddButton(L"Clear", 55, 175, 120, 34, ID_CLEAR);
        AddButton(L"Save History", 190, 175, 135, 34, ID_SAVE);
        AddButton(L"Formula Library", 340, 175, 155, 34, ID_LIBRARY);
        AddButton(L"About", 510, 175, 120, 34, ID_ABOUT);

        AddGroup(L"Detected Category", 30, 245, 300, 80);
        categoryBox = AddReadOnlyEdit(L"", 50, 275, 260, 28, ID_CATEGORY);

        AddGroup(L"Transform Result", 350, 245, 610, 80);
        resultBox = AddReadOnlyEdit(L"", 370, 275, 570, 28, ID_RESULT);

        AddGroup(L"Formula Used", 30, 340, 930, 90);
        formulaBox = AddReadOnlyEdit(L"", 50, 375, 890, 30, ID_FORMULA);

        AddGroup(L"Explanation", 30, 445, 530, 145);
        explanationBox = AddReadOnlyEdit(L"", 50, 475, 490, 90, ID_EXPLANATION, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL);

        AddGroup(L"History List", 580, 445, 380, 145);
        historyList = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"LISTBOX",
            L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
            600,
            475,
            340,
            90,
            hwnd,
            (HMENU)ID_HISTORY,
            GetModuleHandle(nullptr),
            nullptr
        );
        theme.ApplyFont(historyList, monoFont);

        statusBar = CreateWindowEx(
            0,
            L"STATIC",
            L"Ready. Enter a function such as t^2, exp(2t), sin(5t), cosh(3t).",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0,
            635,
            985,
            28,
            hwnd,
            (HMENU)ID_STATUS,
            GetModuleHandle(nullptr),
            nullptr
        );
        theme.ApplyFont(statusBar, normalFont);
    }

    HWND AddLabel(const wchar_t* text, int x, int y, int w, int h, HFONT font)
    {
        HWND control = CreateWindowEx(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
            x, y, w, h, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
        theme.ApplyFont(control, font);
        return control;
    }

    HWND AddGroup(const wchar_t* text, int x, int y, int w, int h)
    {
        HWND control = CreateWindowEx(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            x, y, w, h, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
        theme.ApplyFont(control, normalFont);
        return control;
    }

    HWND AddButton(const wchar_t* text, int x, int y, int w, int h, int id)
    {
        HWND control = CreateWindowEx(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            x, y, w, h, hwnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
        theme.ApplyFont(control, normalFont);
        return control;
    }

    HWND AddEdit(const wchar_t* text, int x, int y, int w, int h, int id)
    {
        HWND control = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", text,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            x, y, w, h, hwnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
        theme.ApplyFont(control, monoFont);
        return control;
    }

    HWND AddReadOnlyEdit(const wchar_t* text, int x, int y, int w, int h, int id, DWORD extraStyle = 0)
    {
        HWND control = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", text,
            WS_CHILD | WS_VISIBLE | ES_READONLY | extraStyle,
            x, y, w, h, hwnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
        theme.ApplyFont(control, monoFont);
        return control;
    }

    wstring GetInput()
    {
        int len = GetWindowTextLength(inputBox);
        vector<wchar_t> buffer(len + 1);
        GetWindowText(inputBox, buffer.data(), len + 1);
        return wstring(buffer.data());
    }

    void SetText(HWND control, const wstring& text)
    {
        SetWindowText(control, text.c_str());
    }

    void SetStatus(const wstring& text)
    {
        SetWindowText(statusBar, text.c_str());
    }

    void HandleCommand(int id)
    {
        switch (id)
        {
        case ID_CALCULATE:
            Calculate(true);
            break;
        case ID_ANALYZE:
            Calculate(false);
            break;
        case ID_CLEAR:
            Clear();
            break;
        case ID_SAVE:
            SaveHistory();
            break;
        case ID_LIBRARY:
            MessageBox(hwnd, FormulaLibrary::Text().c_str(), L"Formula Library", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_ABOUT:
            ShowAbout();
            break;
        }
    }

    bool Calculate(bool addToHistory)
    {
        wstring input = GetInput();
        ParsedFunction pf = parser.Parse(input);

        if (pf.type == FunctionType::Invalid)
        {
            MessageBox(hwnd, pf.error.c_str(), L"LaplaceX Ultimate - Error", MB_OK | MB_ICONERROR);
            SetStatus(L"Error: " + pf.error);
            return false;
        }

        wstring category = engine.Category(pf);
        wstring transform = engine.Transform(pf);
        wstring formula = engine.Formula(pf);
        wstring explanation = engine.Explanation(pf);

        SetText(categoryBox, category);
        SetText(resultBox, transform);
        SetText(formulaBox, formula);
        SetText(explanationBox, explanation);

        if (addToHistory)
        {
            history.Add(TextUtil::Trim(input), transform);
            RefreshHistoryList();
            SetStatus(L"Calculation complete and added to history.");
        }
        else
        {
            SetStatus(L"Analysis complete.");
        }

        return true;
    }

    void RefreshHistoryList()
    {
        SendMessage(historyList, LB_RESETCONTENT, 0, 0);

        for (const auto& e : history.Entries())
        {
            wstring line = e.timestamp + L" | " + e.functionText + L" -> " + e.transform;
            SendMessage(historyList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        }
    }

    void Clear()
    {
        SetText(inputBox, L"");
        SetText(categoryBox, L"");
        SetText(resultBox, L"");
        SetText(formulaBox, L"");
        SetText(explanationBox, L"");
        SetStatus(L"Cleared.");
        SetFocus(inputBox);
    }

    void SaveHistory()
    {
        if (history.Entries().empty())
        {
            MessageBox(hwnd, L"No history entries to save.", L"LaplaceX Ultimate", MB_OK | MB_ICONWARNING);
            SetStatus(L"No history entries to save.");
            return;
        }

        if (history.Save(L"laplace_history.txt"))
        {
            MessageBox(hwnd, L"History saved to laplace_history.txt", L"LaplaceX Ultimate", MB_OK | MB_ICONINFORMATION);
            SetStatus(L"History saved to laplace_history.txt.");
        }
        else
        {
            MessageBox(hwnd, L"Unable to save history file.", L"LaplaceX Ultimate", MB_OK | MB_ICONERROR);
            SetStatus(L"Failed to save history.");
        }
    }

    void ShowAbout()
    {
        wstring about =
            L"LaplaceX Ultimate\n\n"
            L"Developed By\n\n"
            L"T Harshith Krishna Sastry\n"
            L"USN: 1RV25EC223\n\n"
            L"Bhajan Nidinji M\n"
            L"USN: 1RV25EC050\n\n"
            L"Kartik Garg\n"
            L"USN : 1RV25EE025\n\n"
            L"RV College of Engineering\n\n"
            L"Academic Year 2025-2026";

        MessageBox(hwnd, about.c_str(), L"About LaplaceX Ultimate", MB_OK | MB_ICONINFORMATION);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    MainWindow app;

    if (!app.Create(hInstance))
    {
        MessageBox(nullptr, L"Failed to create LaplaceX Ultimate window.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    app.Show(nCmdShow);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
