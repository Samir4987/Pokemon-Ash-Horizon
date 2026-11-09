#pragma once

// Lightweight, data-driven Pokédex foundation.
// Text data is loaded on demand; artwork is deliberately not part of these records.
#include <array>
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <string>
#include <vector>

namespace ash_horizon {

enum class PokemonType : std::uint8_t {
    None, Normal, Fire, Water, Electric, Grass, Ice, Fighting, Poison,
    Ground, Flying, Psychic, Bug, Rock, Ghost, Dragon, Dark, Steel, Fairy
};

inline bool parsePokemonType(const std::string& value, PokemonType& out) {
    static const char* names[] = {
        "none", "normal", "fire", "water", "electric", "grass", "ice",
        "fighting", "poison", "ground", "flying", "psychic", "bug",
        "rock", "ghost", "dragon", "dark", "steel", "fairy"
    };
    for (std::size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (value == names[i]) {
            out = static_cast<PokemonType>(i);
            return true;
        }
    }
    return false;
}

struct PokemonStats {
    std::uint16_t hp = 1;
    std::uint16_t attack = 1;
    std::uint16_t defense = 1;
    std::uint16_t specialAttack = 1;
    std::uint16_t specialDefense = 1;
    std::uint16_t speed = 1;
};

struct SpeciesRecord {
    std::uint16_t nationalDex = 0;
    std::string name;
    PokemonType type1 = PokemonType::Normal;
    PokemonType type2 = PokemonType::None;
    PokemonStats baseStats{};
    std::uint8_t generation = 0; // 1..9; zero means not yet assigned.
};

struct FormRecord {
    std::uint16_t speciesDex = 0;
    std::string formId; // Stable machine ID, e.g. "alola" or "galar".
    std::string displayName;
    std::string region;
    PokemonType type1 = PokemonType::Normal;
    PokemonType type2 = PokemonType::None;
    PokemonStats baseStats{};
    bool overridesStats = false;
};

enum class EvolutionMethod : std::uint8_t {
    Level, Item, Trade, Friendship, Location, Other
};

struct EvolutionRecord {
    std::uint16_t fromDex = 0;
    std::uint16_t toDex = 0;
    EvolutionMethod method = EvolutionMethod::Level;
    std::uint16_t minimumLevel = 1;
    std::string parameter; // Item ID, location ID, or method-specific key.
};

struct EvolutionContext {
    std::uint16_t level = 1;
    std::uint8_t friendship = 0;
    std::string parameter;
};

class PokemonDatabase {
public:
    bool addSpecies(const SpeciesRecord& record) {
        if (record.nationalDex == 0 || record.name.empty() ||
            record.type1 == PokemonType::None || record.baseStats.hp == 0 ||
            record.baseStats.attack == 0 || record.baseStats.defense == 0 ||
            record.baseStats.specialAttack == 0 ||
            record.baseStats.specialDefense == 0 || record.baseStats.speed == 0 ||
            findSpecies(record.nationalDex) != nullptr) return false;
        species_.push_back(record);
        return true;
    }

    bool addForm(const FormRecord& record) {
        if (!findSpecies(record.speciesDex) || record.formId.empty() ||
            record.displayName.empty() || findForm(record.speciesDex, record.formId))
            return false;
        forms_.push_back(record);
        return true;
    }

    bool addEvolution(const EvolutionRecord& record) {
        if (!findSpecies(record.fromDex) || !findSpecies(record.toDex) ||
            record.fromDex == record.toDex || record.minimumLevel < 1)
            return false;
        for (const auto& existing : evolutions_) {
            if (existing.fromDex == record.fromDex &&
                existing.toDex == record.toDex &&
                existing.method == record.method &&
                existing.parameter == record.parameter) return false;
        }
        evolutions_.push_back(record);
        return true;
    }

    const SpeciesRecord* findSpecies(std::uint16_t dex) const {
        for (const auto& record : species_)
            if (record.nationalDex == dex) return &record;
        return nullptr;
    }

    const FormRecord* findForm(std::uint16_t dex, const std::string& formId) const {
        for (const auto& record : forms_)
            if (record.speciesDex == dex && record.formId == formId) return &record;
        return nullptr;
    }

    const std::vector<SpeciesRecord>& species() const { return species_; }
    const std::vector<FormRecord>& forms() const { return forms_; }
    const std::vector<EvolutionRecord>& evolutions() const { return evolutions_; }

    bool canEvolve(const EvolutionRecord& evolution,
                   const EvolutionContext& context) const {
        if (!findSpecies(evolution.fromDex) || !findSpecies(evolution.toDex))
            return false;
        switch (evolution.method) {
        case EvolutionMethod::Level:
            return context.level >= evolution.minimumLevel;
        case EvolutionMethod::Friendship:
            return context.friendship >= 160 && context.level >= evolution.minimumLevel;
        case EvolutionMethod::Item:
        case EvolutionMethod::Trade:
        case EvolutionMethod::Location:
        case EvolutionMethod::Other:
            return !evolution.parameter.empty() &&
                   context.parameter == evolution.parameter;
        }
        return false;
    }

    // File format: first line "ASHDEX<TAB>1"; then eleven tab-separated fields:
    // dex, name, type1, type2 (or "-"), HP, Atk, Def, SpA, SpD, Spe, generation.
    // The loader is bounded by the supplied stream and uses no third-party parser.
    bool loadSpeciesTsv(std::istream& input, std::string* error = nullptr) {
        std::string line;
        if (!std::getline(input, line) || line != "ASHDEX\t1") {
            setError(error, "missing or unsupported ASHDEX header");
            return false;
        }
        std::size_t lineNumber = 1;
        while (std::getline(input, line)) {
            ++lineNumber;
            if (line.empty() || line[0] == '#') continue;
            std::array<std::string, 11> fields{};
            std::size_t start = 0;
            bool validFields = true;
            for (std::size_t i = 0; i < fields.size(); ++i) {
                const std::size_t end = line.find('\t', start);
                if (i + 1 < fields.size() && end == std::string::npos) {
                    validFields = false;
                    break;
                }
                if (i + 1 == fields.size() && end != std::string::npos) {
                    validFields = false;
                    break;
                }
                fields[i] = line.substr(start, end == std::string::npos
                    ? std::string::npos : end - start);
                start = end == std::string::npos ? line.size() : end + 1;
            }
            unsigned long values[7]{};
            if (!validFields || fields[1].empty()) {
                setLineError(error, lineNumber, "expected exactly 11 fields and a name");
                return false;
            }
            for (std::size_t i = 0; i < 7; ++i) {
                if (!parseUnsigned(fields[i == 0 ? 0 : i + 3], values[i])) {
                    setLineError(error, lineNumber, "invalid numeric field");
                    return false;
                }
            }
            unsigned long generation = 0;
            if (!parseUnsigned(fields[10], generation) || generation < 1 || generation > 9) {
                setLineError(error, lineNumber, "generation must be between 1 and 9");
                return false;
            }
            if (values[0] == 0 || values[0] > 1025 ||
                values[1] > 255 || values[2] > 255 || values[3] > 255 ||
                values[4] > 255 || values[5] > 255 || values[6] > 255) {
                setLineError(error, lineNumber, "numeric field out of range");
                return false;
            }
            PokemonType type1, type2 = PokemonType::None;
            if (!parsePokemonType(fields[2], type1) ||
                (fields[3] != "-" && !parsePokemonType(fields[3], type2))) {
                setLineError(error, lineNumber, "unknown type name");
                return false;
            }
            SpeciesRecord record;
            record.nationalDex = static_cast<std::uint16_t>(values[0]);
            record.name = fields[1];
            record.generation = static_cast<std::uint8_t>(generation);
            record.type1 = type1;
            record.type2 = type2;
            record.baseStats = {
                static_cast<std::uint16_t>(values[1]),
                static_cast<std::uint16_t>(values[2]),
                static_cast<std::uint16_t>(values[3]),
                static_cast<std::uint16_t>(values[4]),
                static_cast<std::uint16_t>(values[5]),
                static_cast<std::uint16_t>(values[6])
            };
            if (!addSpecies(record)) {
                setLineError(error, lineNumber, "invalid or duplicate species record");
                return false;
            }
        }
        return true;
    }

private:
    std::vector<SpeciesRecord> species_;
    std::vector<FormRecord> forms_;
    std::vector<EvolutionRecord> evolutions_;

    static bool parseUnsigned(const std::string& text, unsigned long& value) {
        if (text.empty()) return false;
        char* end = nullptr;
        const unsigned long parsed = std::strtoul(text.c_str(), &end, 10);
        if (!end || *end != '\0') return false;
        value = parsed;
        return true;
    }
    static void setError(std::string* error, const char* message) {
        if (error) *error = message;
    }
    static void setLineError(std::string* error, std::size_t line, const char* message) {
        if (error) *error = "line " + std::to_string(line) + ": " + message;
    }
};

} // namespace ash_horizon
