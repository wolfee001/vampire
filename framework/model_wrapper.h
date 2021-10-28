#pragma once

#include "../models.h"
#include "gui_element.h"

class GameDescriptionWrapper : public GameDescription, public GuiElement {
public:
    GameDescriptionWrapper(const GameDescription& wrapped)
        : GameDescription(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mLevelId);
        AddGuiElement(mIsTest);
        AddGuiElement(mGameId);
        AddGuiElement(mMaxTick);
        AddGuiElement(mGrenadeRadius);
        AddGuiElement(mMapSize);
    }
};

class RequestWrapper : public Request, public GuiElement {
public:
    RequestWrapper(const Request& wrapped)
        : Request(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mGameId);
        AddGuiElement(mTick);
        AddGuiElement(mVampireId);
    }
};

class VampireWrapper : public Vampire, public GuiElement {
public:
    VampireWrapper(const Vampire& wrapped)
        : Vampire(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mId);
        AddGuiElement(mX);
        AddGuiElement(mY);
        AddGuiElement(mHealth);
        AddGuiElement(mPlacableGrenades);
        AddGuiElement(mGrenadeRange);
        AddGuiElement(mRunningShoesTick);
    }
};

class GrenadeWrapper : public Grenade, public GuiElement {
public:
    GrenadeWrapper(const Grenade& wrapped)
        : Grenade(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mId);
        AddGuiElement(mX);
        AddGuiElement(mY);
        AddGuiElement(mTick);
        AddGuiElement(mRange);
    }
};

class PowerUpWrapper : public PowerUp, public GuiElement {
public:
    PowerUpWrapper(const PowerUp& wrapped)
        : PowerUp(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mType);
        AddGuiElement(mRemainingTick);
        AddGuiElement(mX);
        AddGuiElement(mY);
    }
};

class BatSquadWrapper : public BatSquad, public GuiElement {
public:
    BatSquadWrapper(const BatSquad& wrapped)
        : BatSquad(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mDensity);
        AddGuiElement(mX);
        AddGuiElement(mY);
    }
};

class EndMessageWrapper : public EndMessage, public GuiElement {
public:
    EndMessageWrapper(const EndMessage& wrapped)
        : EndMessage(wrapped)
    {
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mPoint);
        AddGuiElement(mReason);
    }
};

class TickDescriptionWrapper : public GuiElement {
public:
    TickDescriptionWrapper(const TickDescription& wrapped)
        : mRequest(wrapped.mRequest)
        , mWarnings(wrapped.mWarnings)
        , mMe(wrapped.mMe)
        , mEndMessage(wrapped.mEndMessage)
    {
        for (const auto& element : wrapped.mEnemyVampires) {
            mEnemyVampires.emplace_back(element);
        }

        for (const auto& element : wrapped.mGrenades) {
            mGrenades.emplace_back(element);
        }

        for (const auto& element : wrapped.mPowerUps) {
            mPowerUps.emplace_back(element);
        }

        for (const auto& element : wrapped.mBat1) {
            mBat1.emplace_back(element);
        }

        for (const auto& element : wrapped.mBat2) {
            mBat2.emplace_back(element);
        }

        for (const auto& element : wrapped.mBat3) {
            mBat3.emplace_back(element);
        }

        for (const auto& element : wrapped.mAllBats) {
            mAllBats.emplace_back(element);
        }
    }

    virtual void DrawGui() override
    {
        AddGuiElement(mRequest);
        AddGuiElement(mWarnings);
        AddGuiElement(mMe);
        AddGuiElement(mEnemyVampires);
        AddGuiElement(mGrenades);
        AddGuiElement(mPowerUps);
        AddGuiElement(mBat1);
        AddGuiElement(mBat2);
        AddGuiElement(mBat3);
        AddGuiElement(mAllBats);
        AddGuiElement(mEndMessage);
    }

private:
    RequestWrapper mRequest;
    std::vector<std::string> mWarnings;
    VampireWrapper mMe;
    std::vector<VampireWrapper> mEnemyVampires;
    std::vector<GrenadeWrapper> mGrenades;
    std::vector<PowerUpWrapper> mPowerUps;
    std::vector<BatSquadWrapper> mBat1;
    std::vector<BatSquadWrapper> mBat2;
    std::vector<BatSquadWrapper> mBat3;
    std::vector<BatSquadWrapper> mAllBats;
    EndMessageWrapper mEndMessage;
};
