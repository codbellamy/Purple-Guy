#pragma once
#include "../olcPixelGameEngine.h"

class Item
{
public:
	Item(int, olc::ResourcePack);

public:
	// Determine what the type of the item will be
	enum class Type {
		NONE,
		WEAPON,
		ARMOR,
		CONSUMABLE
	};

public:
	Type getType();
	int getID();
	olc::Decal* getDecal();

protected:
	Type type;
	int id;
	olc::Decal* decal;
};

class Weapon	 : public Item
{
public:
	Weapon(int, olc::ResourcePack);

private:
	float damage;
	int statusFlags;

public:
	// Bitmask to manipulate the status effects of the weapon
	enum StatusEffect
	{
		POISON   = 0b1,
		FIRE     = 0b10,
		ACID     = 0b100,
		STRENGTH = 0b1000
	};

public:
	float getDamage();
	bool hasStatus(StatusEffect);

	void addStatusEffects(int statusEffects);
	void removeStatusEffects(int statusEffects);

};
class Armor		 : public Item
{
public:
	Armor(int, olc::ResourcePack);

private:
	float armor;
	int statusFlags;

public:
	enum StatusEffect
	{
		DOUBLE_ARMOR  = 0b1,
		HARDNESS      = 0b01,
		THORN         = 0b001,
		RESIST_POISON = 0b0001,
		RESIST_HEAT   = 0b00001
	};

public:
	float getArmor();
	bool hasStatus(StatusEffect);

	void addStatusEffects(int statusEffects);
	void removeStatusEffects(int statusEffects);
};
class Consumable : public Item
{
public:
	Consumable();

private:
	float amount;
	
};