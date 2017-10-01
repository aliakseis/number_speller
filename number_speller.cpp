// number_speller.cpp : Defines the entry point for the console application.
//

#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <math.h>
#include <utility>

#ifdef _WIN32
#include <fcntl.h>  
#include <io.h>
#else
#include <locale>
#endif

using std::wstring;
using std::make_shared;

using std::wcout;
using std::cin;


///////////////////////////////////////////////////////////////////////////////

struct ISpeller;

typedef std::shared_ptr<ISpeller> SpellerPtr;

class SpellerInput
{
public:
    SpellerInput(int number) 
        : m_number(abs(number)), m_negative(number < 0)  {}
    SpellerInput(unsigned long long number)
        : m_number(number), m_negative(false)  {}

    SpellerInput left(int pos) const
    {
        unsigned long long divider = 1;
        for (int i = 0; i < pos; ++i)
            divider *= 10;

        SpellerInput result(*this);
        result.m_number = m_number / divider;
        return result;
    }
    SpellerInput right(int pos) const
    {
        unsigned long long divider = 1;
        for (int i = 0; i < pos; ++i)
            divider *= 10;

        SpellerInput result(*this);
        result.m_number = m_number % divider;
        return result;
    }

    SpellerInput addSubstitute(const SpellerPtr& from, const SpellerPtr& to) const
    {
        SpellerInput result(*this);
        result.m_substitutes[from] = to;
        return result;
    }

    bool isLessEqual(unsigned int threshold) const { return m_number <= threshold; }
    bool isNegative() const { return m_negative; }

    wstring spell(const SpellerPtr& speller) const;
private:
    unsigned long long m_number;
    bool m_negative;
    typedef std::map<SpellerPtr, SpellerPtr> SpellerPtrMap;
    SpellerPtrMap m_substitutes;
};


struct ISpeller
{
    virtual ~ISpeller() {}
    virtual wstring doSpell(const SpellerInput&) const = 0;
};


inline wstring SpellerInput::spell(const SpellerPtr& speller) const
{
    SpellerPtrMap::const_iterator it = m_substitutes.find(speller);
    return (m_substitutes.end() == it)
        ? speller->doSpell(*this) 
        : it->second->doSpell(*this);
}

///////////////////////////////////////////////////////////////////////////////

class NullSpeller : public ISpeller
{
public:
    virtual wstring doSpell(const SpellerInput& ) const override
    {
        return wstring();
    }
};

class StringSpeller : public ISpeller
{
public:
    explicit StringSpeller(const wstring& value) : m_value(value) {}
    virtual wstring doSpell(const SpellerInput& ) const override
    {
        return m_value + L' ';
    }
private:
    wstring m_value;
};

class SubstituteSpeller : public ISpeller
{
public:
    SubstituteSpeller(const SpellerPtr& from, const SpellerPtr& to, const SpellerPtr& what)
        : m_from(from), m_to(to), m_what(what)
    {
    }

    virtual wstring doSpell(const SpellerInput& input) const override
    {
        return input.addSubstitute(m_from, m_to).spell(m_what);
    }

private:
    SpellerPtr m_from, m_to, m_what;
};

class SelectionSpeller : public ISpeller
{
public:
    SelectionSpeller(unsigned int threshold, const SpellerPtr& lower, const SpellerPtr& higher)
        : m_lower(lower), m_higher(higher), m_threshold(threshold)
    {
    }

    virtual wstring doSpell(const SpellerInput& input) const override
    {
        return input.isLessEqual(m_threshold)? input.spell(m_lower) : input.spell(m_higher);
    }

private:
    SpellerPtr m_lower, m_higher;
    unsigned int m_threshold;
};

class SignSpeller : public ISpeller
{
public:
    SignSpeller(const SpellerPtr& higher, const SpellerPtr& lower)
        : m_lower(lower), m_higher(higher)
    {
    }

    virtual wstring doSpell(const SpellerInput& input) const override
    {
        return input.isNegative()? input.spell(m_lower) : input.spell(m_higher);
    }

private:
    SpellerPtr m_lower, m_higher;
};

class SequentialSpeller : public ISpeller
{
public:
    SequentialSpeller(const SpellerPtr& first, const SpellerPtr& second)
        : m_first(first), m_second(second)
    {
    }

    virtual wstring doSpell(const SpellerInput& input) const override
    {
        return input.spell(m_first) + input.spell(m_second);
    }

private:
    SpellerPtr m_first, m_second;
};

class PositionSplitSpeller : public ISpeller
{
public:
    PositionSplitSpeller(
        int position,
        const SpellerPtr& right, 
        const SpellerPtr& left, 
        bool inverseOrder = false)
    : m_left(left), m_right(right), m_position(position), m_inverseOrder(inverseOrder)
    {
    }

    virtual wstring doSpell(const SpellerInput& input) const override
    {
        wstring left = input.left(m_position).spell(m_left);
        wstring right = input.right(m_position).spell(m_right);

        return m_inverseOrder? right + left : left + right;
    }

private:
    SpellerPtr m_left, m_right;
    int m_position;
    bool m_inverseOrder;
};

template<typename... Args>
auto select(unsigned int threshold, const SpellerPtr& p, Args... xs)
{
    return make_shared<SelectionSpeller>(threshold, p, select(xs...));
}

template <typename T>
inline decltype(auto) select(T&& p)
{
    return std::forward<T>(p);
}

///////////////////////////////////////////////////////////////////////////////

SpellerPtr getEnglishSpeller()
{
    SpellerPtr unit = make_shared<NullSpeller>();

    SpellerPtr units
        = make_shared<SequentialSpeller>(select(
            0, make_shared<NullSpeller>(),
            1, make_shared<StringSpeller>(L"One"),
            2, make_shared<StringSpeller>(L"Two"),
            3, make_shared<StringSpeller>(L"Three"),
            4, make_shared<StringSpeller>(L"Four"),
            5, make_shared<StringSpeller>(L"Five"),
            6, make_shared<StringSpeller>(L"Six"),
            7, make_shared<StringSpeller>(L"Seven"),
            8, make_shared<StringSpeller>(L"Eight"),
            make_shared<StringSpeller>(L"Nine")),
        unit);

    SpellerPtr teens = select(
        10, make_shared<StringSpeller>(L"Ten"),
        11, make_shared<StringSpeller>(L"Eleven"),
        12, make_shared<StringSpeller>(L"Twelve"),
        13, make_shared<StringSpeller>(L"Thirteen"),
        14, make_shared<StringSpeller>(L"Fourteen"),
        15, make_shared<StringSpeller>(L"Fifteen"),
        16, make_shared<StringSpeller>(L"Sixteen"),
        17, make_shared<StringSpeller>(L"Seventeen"),
        18, make_shared<StringSpeller>(L"Eighteen"),
        make_shared<StringSpeller>(L"Nineteen"));

    SpellerPtr tens = select(
        2, make_shared<StringSpeller>(L"Twenty"),
        3, make_shared<StringSpeller>(L"Thirty"),
        4, make_shared<StringSpeller>(L"Forty"),
        5, make_shared<StringSpeller>(L"Fifty"),
        6, make_shared<StringSpeller>(L"Sixty"),
        7, make_shared<StringSpeller>(L"Seventy"),
        8, make_shared<StringSpeller>(L"Eighty"),
        make_shared<StringSpeller>(L"Ninety"));

    SpellerPtr tensAndUnits
        = make_shared<SelectionSpeller>(9, units
        , make_shared<SelectionSpeller>(19, make_shared<SequentialSpeller>(teens, unit)
        , make_shared<PositionSplitSpeller>(1, units, tens)));

    SpellerPtr hundreds 
        = make_shared<SelectionSpeller>(0
        , make_shared<NullSpeller>()
        , make_shared<SubstituteSpeller>(unit
        , make_shared<StringSpeller>(L"Hundred"), units));


    SpellerPtr triad = make_shared<PositionSplitSpeller>(2, tensAndUnits, hundreds);

    SpellerPtr thousands 
        = make_shared<SelectionSpeller>(0, make_shared<NullSpeller>()
        , make_shared<SubstituteSpeller>(unit, make_shared<StringSpeller>(L"Thousand")
        , triad));

    SpellerPtr millions 
        = make_shared<SelectionSpeller>(0, make_shared<NullSpeller>()
        , make_shared<SubstituteSpeller>(unit, make_shared<StringSpeller>(L"Million")
        , triad));

    SpellerPtr billions 
        = make_shared<SelectionSpeller>(0, make_shared<NullSpeller>()
        , make_shared<SubstituteSpeller>(unit, make_shared<StringSpeller>(L"Billion")
        , triad));

    SpellerPtr positive
        = make_shared<PositionSplitSpeller>(3, triad
        , make_shared<PositionSplitSpeller>(3, thousands
        , make_shared<PositionSplitSpeller>(3, millions
        , billions)));

    SpellerPtr result
        = make_shared<SignSpeller>(make_shared<SelectionSpeller>(0, make_shared<StringSpeller>(L"Zero"), positive)
        , make_shared<SequentialSpeller>(make_shared<StringSpeller>(L"Minus"), positive));

    return result;
}

SpellerPtr getRussianSpeller()
{
    SpellerPtr one = make_shared<StringSpeller>(L"один");
    SpellerPtr two = make_shared<StringSpeller>(L"два");

#if 0
    SpellerPtr unit = make_shared<StringSpeller>(L"юнит");
    SpellerPtr unitGen = make_shared<StringSpeller>(L"юнита");
    SpellerPtr unitGenPl = make_shared<StringSpeller>(L"юнитов");
#else
    SpellerPtr unit = make_shared<NullSpeller>();
    SpellerPtr unitGen = make_shared<NullSpeller>();
    SpellerPtr unitGenPl = make_shared<NullSpeller>();
#endif

    SpellerPtr unitsMoreThan4 = select(
        5, make_shared<StringSpeller>(L"пять"),
        6, make_shared<StringSpeller>(L"шесть"),
        7, make_shared<StringSpeller>(L"семь"),
        8, make_shared<StringSpeller>(L"восемь"),
        make_shared<StringSpeller>(L"девять"));

    SpellerPtr units = select(
        0, unitGenPl,
        1, make_shared<SequentialSpeller>(one, unit),
        2, make_shared<SequentialSpeller>(two, unitGen),
        3, make_shared<SequentialSpeller>(make_shared<StringSpeller>(L"три"), unitGen),
        4, make_shared<SequentialSpeller>(make_shared<StringSpeller>(L"четыре"), unitGen),
        make_shared<SequentialSpeller>(unitsMoreThan4, unitGenPl));

    SpellerPtr teens = select(
        10, make_shared<StringSpeller>(L"десять"),
        11, make_shared<StringSpeller>(L"одиннадцать"),
        12, make_shared<StringSpeller>(L"двенадцать"),
        13, make_shared<StringSpeller>(L"тринадцать"),
        14, make_shared<StringSpeller>(L"четырнадцать"),
        15, make_shared<StringSpeller>(L"пятнадцать"),
        16, make_shared<StringSpeller>(L"шестнадцать"),
        17, make_shared<StringSpeller>(L"семнадцать"),
        18, make_shared<StringSpeller>(L"восемнадцать"),
        make_shared<StringSpeller>(L"девятнадцать"));

    SpellerPtr tens = select(
        2, make_shared<StringSpeller>(L"двадцать"),
        3, make_shared<StringSpeller>(L"тридцать"),
        4, make_shared<StringSpeller>(L"сорок"),
        5, make_shared<StringSpeller>(L"пятьдесят"),
        6, make_shared<StringSpeller>(L"шестьдесят"),
        7, make_shared<StringSpeller>(L"семьдесят"),
        8, make_shared<StringSpeller>(L"восемьдесят"),
        make_shared<StringSpeller>(L"девяносто"));

    SpellerPtr tensAndUnits
        = make_shared<SelectionSpeller>(9, units
        , make_shared<SelectionSpeller>(19, make_shared<SequentialSpeller>(teens, unitGenPl)
            , make_shared<PositionSplitSpeller>(1, units, tens)));

    SpellerPtr hundreds = select(
        0, make_shared<NullSpeller>(),
        1, make_shared<StringSpeller>(L"сто"),
        2, make_shared<StringSpeller>(L"двести"),
        3, make_shared<StringSpeller>(L"триста"),
        4, make_shared<StringSpeller>(L"четыреста"),
        5, make_shared<StringSpeller>(L"пятьсот"),
        6, make_shared<StringSpeller>(L"шестьсот"),
        7, make_shared<StringSpeller>(L"семьсот"),
        8, make_shared<StringSpeller>(L"восемьсот"),
        make_shared<StringSpeller>(L"девятьсот"));

    SpellerPtr triad = make_shared<PositionSplitSpeller>(2, tensAndUnits, hundreds);

    SpellerPtr thousands 
        = make_shared<SelectionSpeller>(0, make_shared<NullSpeller>()
            , make_shared<SubstituteSpeller>(one, make_shared<StringSpeller>(L"одна")
            , make_shared<SubstituteSpeller>(two, make_shared<StringSpeller>(L"две")
            , make_shared<SubstituteSpeller>(unit, make_shared<StringSpeller>(L"тысяча")
            , make_shared<SubstituteSpeller>(unitGen, make_shared<StringSpeller>(L"тысячи")
            , make_shared<SubstituteSpeller>(unitGenPl, make_shared<StringSpeller>(L"тысяч")
            , triad))))));

    SpellerPtr millions 
        = make_shared<SelectionSpeller>(0, make_shared<NullSpeller>()
            , make_shared<SubstituteSpeller>(unit, make_shared<StringSpeller>(L"миллион")
            , make_shared<SubstituteSpeller>(unitGen, make_shared<StringSpeller>(L"миллиона")
            , make_shared<SubstituteSpeller>(unitGenPl, make_shared<StringSpeller>(L"миллионов")
            , triad))));

    SpellerPtr billions 
        = make_shared<SelectionSpeller>(0, make_shared<NullSpeller>()
            , make_shared<SubstituteSpeller>(unit, make_shared<StringSpeller>(L"миллиард")
            , make_shared<SubstituteSpeller>(unitGen, make_shared<StringSpeller>(L"миллиарда")
            , make_shared<SubstituteSpeller>(unitGenPl, make_shared<StringSpeller>(L"миллиардов")
            , triad))));

    SpellerPtr positive
        = make_shared<PositionSplitSpeller>(3, triad
        , make_shared<PositionSplitSpeller>(3, thousands
        , make_shared<PositionSplitSpeller>(3, millions
            , billions)));

    SpellerPtr result
        = make_shared<SignSpeller>(make_shared<SelectionSpeller>(0, make_shared<StringSpeller>(L"ноль"), positive)
            , make_shared<SequentialSpeller>(make_shared<StringSpeller>(L"минус"), positive));

    return result;
}


int main(int /*argc*/, char* /*argv*/[])
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_U16TEXT);
#else
    std::locale::global(std::locale("en_US.UTF-8"));
#endif
    SpellerPtr englishSpeller = getEnglishSpeller();
    SpellerPtr russianSpeller = getRussianSpeller();

    int value = 0;
    while (cin >> value)
    {
        wcout << englishSpeller->doSpell(value) << '\n';
        wcout << russianSpeller->doSpell(value) << '\n';
    }

	return 0;
}
