#pragma once
#include "../Main/aiUnit.h"
#include "../Main/wUnitServer.h"
#include <stdexcept>

namespace NAI {
// An unexpected call fails immediately instead of using an incomplete world.
// Pure interface boilerplate keeps focused inventory/reaction tests independent
// of proprietary game resources and graphics initialization.
class AIUnitFixture: public IAIUnit
{
    OBJECT_NOCOPY_METHODS(AIUnitFixture);
public:
    CPtr<NWorld::CUnitServer> server;
    bool underAIControl = true;
    int coverCalls = 0;
    NWorld::CUnitServer* GetUnitServer() const override { return server; }
    NRPG::IUnitMission* GetUnitMission() const override { throw std::runtime_error("unexpected AI fixture operation"); }
    NRPG::CUnit* GetRPGUnit() const override { throw std::runtime_error("unexpected AI fixture operation"); }
    void Synchronize(bool = true) override { throw std::runtime_error("unexpected AI fixture operation"); }
    SPosition GetPosition() override { throw std::runtime_error("unexpected AI fixture operation"); }
    SUnitPosition GetUnitPosition() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetPosition( SPosition _pPosition ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetPosition( SPathPlace _pPosition ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    SPosition GetPrevPosition() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SavePrevPosition() override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetTurnStartHP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void GetHP( int *_nHP, int *_nMaxHP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetHP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetHP( int _nHP, int _nMaxHP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    void GetAP( int *_nAP, int *_nMaxAP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetAP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetAP( int _nAP, int _nMaxAP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetMaxAP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetMaxHP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SpendHP( int _nHP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SpendAP( int _nAP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetPose( int pose ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    bool IsUsingCannon() override { throw std::runtime_error("unexpected AI fixture operation"); }
    NWorld::CCannon *GetCannon() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetCannon( NWorld::CCannon *_pCannon ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    bool IsMovedThisTurn() override { throw std::runtime_error("unexpected AI fixture operation"); }
    bool IsDead() override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetRemainAP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetToHit( IAIUnit *pTarget, const NAI::SUnitPosition &pos, NAI::EHitLocation hl = NAI::HL_ANY ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    bool IsPerformingAction() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void OnTurnStarted() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void GetLastSeenEnemy( SPosition *Position, IAIUnit **ppAIUnit ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetLastSeenEnemy( IAIUnit *pAIUnit ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    CAIInventory* GetAIInventory() override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetHurtHP() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetHurtHP( int _nHurtHP ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetCoverForFixedUnit( const NAI::SUnitPosition &pos, NWorld::CUnitServer *pTarget, NRPG::CWeaponItem *pWeaponItem, NAI::EHitLocation HitLocation ) override { ++coverCalls; if (!IsValid(pWeaponItem)) throw std::runtime_error("invalid AI weapon reached cover scoring"); return 0; }
    bool HasInactivePose() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void AssignControl( IAIControl *pAIControl ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    CTask* GetRoute() const override { throw std::runtime_error("unexpected AI fixture operation"); }
    void ActivateCurrentControl() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void DeactivateCurrentControl() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void OnControlFinished() override { throw std::runtime_error("unexpected AI fixture operation"); }
    bool IsUnderAIControl() override { return underAIControl; }
    int GetAdditionalExpediency() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetAdditionalExpediency( int nExpediency ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetMaxToHit( int _nMaxToHit ) override { throw std::runtime_error("unexpected AI fixture operation"); }
    int GetMaxToHit() override { throw std::runtime_error("unexpected AI fixture operation"); }
    bool HasVisibleEnemies() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void DebugOutput() override { throw std::runtime_error("unexpected AI fixture operation"); }
    void OnDied() override { throw std::runtime_error("unexpected AI fixture operation"); }
    IAILogic* GetLogic() const override { throw std::runtime_error("unexpected AI fixture operation"); }
    void SetLogic( IAILogic *_pLogic ) override { throw std::runtime_error("unexpected AI fixture operation"); }
};
} // namespace NAI
