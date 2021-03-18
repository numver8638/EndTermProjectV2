#pragma once

#include <vector>
#include <utility>

#include "Position.h"

// 블록의 종류를 나타내는 열거형.
enum class BlockKind {
	Type1,	// 4x1	○ □ ■ ◆
	Type2,	// 1x1 □
	Type3,	// 2x2    ○ □   
	Type4	// ㅗ □   ◆ ■
			//  □ ■ □
};

// 블록의 방향을 나타내는 열거형.
enum class BlockDirection {
	Vertical,
	Horizontal
};

struct BlockData {
	int Width;
	int Height;
	std::vector<std::pair<Position, wchar_t>> Data;
};

// 블록을 나타내는 클래스.
class Block {
private:
	BlockKind m_kind;
	BlockDirection m_direction;
	Position m_pos;

public:
	Block() = default;

	Block(BlockKind kind, BlockDirection direction, Position pos)
		: m_kind(kind), m_direction(direction), m_pos(pos) {}

	// 블록의 종류를 반환하는 메서드.
	BlockKind GetKind() const { return m_kind; }

	// 블록의 위치를 반환하는 메서드.
	Position GetPosition() const { return m_pos; }

	// 블록의 절대적 위치를 설정하는 메서드.
	void SetAbsolutePosition(Position pos) { m_pos = pos; }

	// 블럭의 현재 위치를 중심으로 상대적 위치를 설정하는 메서드.
	void SetRelativePosition(Position delta) {
		m_pos.X += delta.X;
		m_pos.Y += delta.Y;
	}

	// 블록의 방향을 반환하는 메서드.
	BlockDirection GetDirection() const { return m_direction; }

	// 블록의 방향을 설정하는 메서드.
	void SetDirection(BlockDirection direction) { m_direction = direction; }

	// 블록 데이터를 가져오는 메서드.
	const BlockData& GetData() const { return GetData(m_kind, m_direction); }

	// 종류와 방향에 따라 사전 정의된 블록 데이터(모양)을 가져오는 정적 메서드.
	static const BlockData& GetData(BlockKind kind, BlockDirection direction);

	// 랜덤 종류, 랜덤 방향으로 블럭을 생성하는 메서드.
	static Block GenerateRandomBlock(bool verticalOnly = false);
};