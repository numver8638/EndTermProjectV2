#include "Block.h"

#include <cstdlib>

// static
const BlockData& Block::GetData(BlockKind kind, BlockDirection direction) {
	// 타입/방향별 블록 데이터. 편의를 위해 사전 정의.
	static BlockData DataTable[] = {
		{ // Type1, Vertical
			1, 4,
			{
				{ Position(0, 0), L'○' },
				{ Position(0, -1), L'□' },
				{ Position(0, -2), L'■' },
				{ Position(0, -3), L'◆' },
			}
		},
		{ // Type1, Horizontal
			4, 1,
			{
				{ Position(0, 0), L'○' },
				{ Position(1, 0), L'□' },
				{ Position(2, 0), L'■' },
				{ Position(3, 0), L'◆' },
			}
		},
		{ // Type2, Vertical
			1, 1,
			{
				{ Position(0, 0), L'□' }
			}
		},
		{ // Type2, Horizontal
			1, 1,
			{
				{ Position(0, 0), L'□' }
			}
		},
		{ // Type3, Vertical
			2, 2,
			{
				{ Position(0, 0), L'◆' },
				{ Position(1, 0), L'■' },
				{ Position(0, -1), L'○' },
				{ Position(1, -1), L'□' },
			}
		},
		{ // Type3, Horizontal
			2, 2,
			{
				{ Position(0, 0), L'◆' },
				{ Position(1, 0), L'■' },
				{ Position(0, -1), L'○' },
				{ Position(1, -1), L'□' },
			}
		},
		{ // Type4, Vertical
			3, 2,
			{
				{ Position(0, 0), L'□' },
				{ Position(1, 0), L'■' },
				{ Position(2, 0), L'□' },
				{ Position(1, -1), L'□' },
			}
		},
		{ // Type4, Horizontal
			3, 2,
			{
				{ Position(0, -1), L'□' },
				{ Position(1, -1), L'■' },
				{ Position(2, -1), L'□' },
				{ Position(1, 0), L'□' },
			}
		},
	};

	return DataTable[(static_cast<size_t>(kind) * 2) + static_cast<size_t>(direction)];
}

// static
Block Block::GenerateRandomBlock(bool verticalOnly) {
	BlockKind kind = static_cast<BlockKind>(rand() % 4);
	BlockDirection direction = verticalOnly ? BlockDirection::Vertical : static_cast<BlockDirection>(rand() % 2);

	return Block(kind, direction, Position());
}