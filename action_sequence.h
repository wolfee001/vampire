#pragma once

#include "models.h"
#include <cmath>
#include <functional>
#include <optional>
#include <stdexcept>

class ActionSequence {
public:
    ActionSequence() = default;
    using ActionSequence_t = uint16_t;

    explicit ActionSequence(const ActionSequence_t sequence)
        : mMove(static_cast<uint8_t>(sequence >> 8))
        , mBombing(static_cast<uint8_t>(sequence & 0xFF))
    {
        if (mMove > MaxMoveId) {
            throw std::runtime_error("Too big ActionSequenceId");
        }
        if (mBombing > MaxBombingId) {
            throw std::runtime_error("Too big bombing");
        }
    }

    explicit ActionSequence(const Answer& answer)
    {
        const auto getMoveIndex = [](const char step) -> uint8_t {
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

        mMove = std::invoke([&answer]() -> uint8_t {
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
            mMove = static_cast<uint8_t>(mMove + getMoveIndex(answer.mSteps[i]) * std::pow(4, i));
        }

        if (answer.mPlaceGrenade) {
            mBombing = 1;
        } else {
            if (answer.mThrow) {
                switch (answer.mThrow->mDirection) {
                case Throw::Direction::Up:
                    mBombing = 2;
                    break;
                case Throw::Direction::Down:
                    mBombing = 3;
                    break;
                case Throw::Direction::Left:
                    mBombing = 4;
                    break;
                case Throw::Direction::Right:
                    mBombing = 5;
                    break;
                case Throw::Direction::XUp:
                    mBombing = 6;
                    break;
                case Throw::Direction::XDown:
                    mBombing = 7;
                    break;
                case Throw::Direction::XLeft:
                    mBombing = 8;
                    break;
                case Throw::Direction::XRight:
                    mBombing = 9;
                    break;
                }
            } else {
                mBombing = 0;
            }
        }
    }

    bool operator==(const ActionSequence& other) const
    {
        return mMove == other.mMove && mBombing == other.mBombing;
    }

    [[nodiscard]] ActionSequence_t GetId() const
    {
        return static_cast<ActionSequence_t>((ActionSequence_t(mMove) << 8) | static_cast<ActionSequence_t>(mBombing));
    }

    [[nodiscard]] bool IsGrenade() const
    {
        return mBombing == 1;
    }

    [[nodiscard]] bool IsThrow() const
    {
        return mBombing > 1;
    }

    [[nodiscard]] uint8_t GetNumberOfSteps() const
    {
        if (mMove == 0) {
            return 0;
        }
        if (mMove < 5) {
            return 1;
        }
        if (mMove < 21) {
            return 2;
        }

        return 3;
    }

    [[nodiscard]] uint8_t GetNthStep(int n) const
    {
        auto moves = mMove;
        if (moves < 5) {
            moves = static_cast<uint8_t>(moves - 1);
        } else if (moves < 21) {
            moves = static_cast<uint8_t>(moves - 5);
        } else {
            moves = static_cast<uint8_t>(moves - 21);
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

    [[nodiscard]] Throw::Direction GetThrowDirection() const
    {
        switch (mBombing) {
        case 2:
            return Throw::Direction::Up;
        case 3:
            return Throw::Direction::Down;
        case 4:
            return Throw::Direction::Left;
        case 5:
            return Throw::Direction::Right;
        case 6:
            return Throw::Direction::XUp;
        case 7:
            return Throw::Direction::XDown;
        case 8:
            return Throw::Direction::XLeft;
        case 9:
            return Throw::Direction::XRight;
        default:
            throw std::runtime_error("invalid throw");
        }
    }

    Answer GetAnswer() const
    {
        Answer answer;
        answer.mPlaceGrenade = IsGrenade();

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

        const size_t size = std::invoke([&mMove = mMove]() -> size_t {
            if (mMove == 0) {
                return 0;
            }
            if (mMove < 5) {
                return 1;
            }
            if (mMove < 21) {
                return 2;
            }
            return 3;
        });

        uint8_t moves = mMove;
        if (size == 1) {
            moves = static_cast<uint8_t>(moves - 1);
        } else if (size == 2) {
            moves = static_cast<uint8_t>(moves - 5);
        } else if (size == 3) {
            moves = static_cast<uint8_t>(moves - 21);
        }

        answer.mSteps.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            answer.mSteps.push_back(indexToStep(moves % 4));
            moves /= 4;
        }

        if (!IsThrow()) {
            return answer;
        }

        answer.mThrow = std::make_optional<Throw>();

        switch (mBombing) {
        case 2:
            answer.mThrow->mDirection = Throw::Direction::Up;
            break;
        case 3:
            answer.mThrow->mDirection = Throw::Direction::Down;
            break;
        case 4:
            answer.mThrow->mDirection = Throw::Direction::Left;
            break;
        case 5:
            answer.mThrow->mDirection = Throw::Direction::Right;
            break;
        case 6:
            answer.mThrow->mDirection = Throw::Direction::XUp;
            break;
        case 7:
            answer.mThrow->mDirection = Throw::Direction::XDown;
            break;
        case 8:
            answer.mThrow->mDirection = Throw::Direction::XLeft;
            break;
        case 9:
            answer.mThrow->mDirection = Throw::Direction::XRight;
            break;
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

    bool GetNextId()
    {
        if (mBombing == MaxBombingId) {
            mBombing = 0;

            if (mMove == MaxMoveId) {
                return false;
            }
            ++mMove;
        } else {
            ++mBombing;
        }

        return true;
    }

    static const constexpr uint8_t MaxMoveId = 21 + 3 * (1) + 3 * (4) + 3 * (4 * 4); // right, right, right
    static const constexpr uint8_t MaxBombingId = 9;

    static const constexpr ActionSequence_t IdCount = MaxMoveId * MaxBombingId;

private:
    uint8_t mMove = 0;
    uint8_t mBombing = 0;
};