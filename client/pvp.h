#ifndef __PVP_H__
#define __PVP_H__

#include <unordered_map>
#include <string>

enum AttackType {
  None = 0,
  SwordDash = 1,
  SwordThrow = 2,
  GunShot = 4,
  TsExplosion = 8,
  TsImpale = 16,
  Ts = 0x18,
  Everything = 0x1f 
};

std::unordered_map<AttackType, std::string> attack_names = {
  {AttackType::None, "None"},
  {AttackType::SwordDash, "Sword Dash"},
  {AttackType::SwordThrow, "Sword Throw"},
  {AttackType::GunShot, "Gun Shot"},
  {AttackType::TsExplosion, "TS Explosion"},
  {AttackType::TsImpale, "TS Impale"},
  {AttackType::Ts, "TS"},
  {AttackType::Everything, "Everything"}
};


/*
enum Team {
  None,
  Trainee,
  Garrison,
  SurveyCorps,
  MIlitaryPolice,
  Infected,
  Hunted
};
*/
#endif