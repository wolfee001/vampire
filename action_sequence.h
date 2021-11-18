#pragma once

#include "models.h"
#include <cmath>
#include <functional>
#include <optional>
#include <stdexcept>

class ActionSequence {
public:
    ActionSequence() = default;
    using ActionSequence_t = uint8_t;

    explicit ActionSequence(const ActionSequence_t sequence)
        : mSequence(sequence)
    {
        if (mSequence > MaxSequenceId) {
            throw std::runtime_error("Too big ActionSequenceId");
        }
    }

    explicit ActionSequence(const Answer& answer)
    {
        const auto getMoveIndex = [](const char step) -> ActionSequence_t {
            switch (step) {
            case 'U':
                return 0;
            case 'D':
                return 1;
            case 'L':
                return 2;
            case 'R':
                return 3;
            default:
                throw std::runtime_error("Invalid step");
            }
        };

        mSequence = std::invoke([&answer]() -> ActionSequence_t {
            switch (answer.mSteps.size()) {
            case 0:
                return 0; // empty
            case 1:
                return 1; // 1-4 (4^1)
            case 2:
                return 5; // 5-20 (4^2)
            case 3:
                return 21; // 21-84 (4^3)
            default:
                throw std::runtime_error("invalid size");
            }
        });

        for (size_t i = 0; i < answer.mSteps.size(); ++i) {
            mSequence = static_cast<ActionSequence_t>(mSequence + getMoveIndex(answer.mSteps[i]) * std::pow(4, i));
        }

        mSequence = static_cast<ActionSequence_t>((mSequence << 1) | (answer.mPlaceGrenade ? 1 : 0));
    }

    bool operator==(const ActionSequence& other) const
    {
        return mSequence == other.mSequence;
    }

    [[nodiscard]] ActionSequence_t GetId() const
    {
        return mSequence;
    }

    [[nodiscard]] bool IsGrenade() const
    {
        return static_cast<bool>(mSequence & 1);
    }

    [[nodiscard]] uint8_t GetNumberOfSteps() const
    {
        const auto moves = static_cast<ActionSequence_t>(mSequence >> 1);

        if (moves == 0) {
            return 0;
        }
        if (moves < 5) {
            return 1;
        }
        if (moves < 21) {
            return 2;
        }

        return 3;
    }

    [[nodiscard]] uint8_t GetNthStep(int n) const
    {
        auto moves = static_cast<ActionSequence_t>(mSequence >> 1);

        if (moves < 5) {
            moves = static_cast<ActionSequence_t>(moves - 1);
        } else if (moves < 21) {
            moves = static_cast<ActionSequence_t>(moves - 5);
        } else {
            moves = static_cast<ActionSequence_t>(moves - 21);
        }

        switch (n) {
        case 0:
            return static_cast<uint8_t>(moves % 4);
        case 1:
            return static_cast<uint8_t>((moves / 4) % 4);
        case 2:
            return static_cast<uint8_t>((moves / (4 * 4)) % 4);
        }

        return 255;
    }

    std::optional<Throw> GetThrow() const
    {
        // TODO: Gabor!!!
        return std::nullopt;
    }

    Answer GetAnswer() const
    {
        Answer answer;
        answer.mPlaceGrenade = static_cast<bool>(mSequence & 1);

        const auto indexToStep = [](const ActionSequence_t step) -> char {
            switch (step) {
            case 0:
                return 'U';
            case 1:
                return 'D';
            case 2:
                return 'L';
            case 3:
                return 'R';
            default:
                throw std::runtime_error("Invalid step");
            }
        };

        ActionSequence_t moves = static_cast<ActionSequence_t>(mSequence >> 1);

        const size_t size = std::invoke([&moves]() -> size_t {
            if (moves == 0) {
                return 0;
            } else if (moves < 5) {
                return 1;
            } else if (moves < 21) {
                return 2;
            } else {
                return 3;
            }
        });

        if (size == 1) {
            moves = static_cast<ActionSequence_t>(moves - 1);
        } else if (size == 2) {
            moves = static_cast<ActionSequence_t>(moves - 5);
        } else if (size == 3) {
            moves = static_cast<ActionSequence_t>(moves - 21);
        }

        answer.mSteps.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            answer.mSteps.push_back(indexToStep(moves % 4));
            moves /= 4;
        }

        return answer;
    }

    void Print() const
    {
        const auto answer = GetAnswer();
        std::cerr << "grenade: " << answer.mPlaceGrenade << " moves: ";
        for (const auto& step : answer.mSteps) {
            std::cerr << step;
        }
        std::cerr << std::endl;
    }

    static const constexpr ActionSequence_t MaxSequenceId = ((21 + 3 * (1) + 3 * (4) + 3 * (4 * 4)) << 1) + 1; // bomb, right, right, right

private:
    ActionSequence_t mSequence = 0;
};