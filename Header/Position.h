#pragma once

// 좌표를 나타내는 구조체.
struct Position {
	int X;
	int Y;

	Position()
		: X(0), Y(0) {}

	Position(int x, int y)
		: X(x), Y(y) {}

	// 편의를 위해 +=, + 연산자만 정의함.
	Position& operator +=(const Position& rhs) {
		X += rhs.X;
		Y += rhs.Y;

		return *this;
	}
};

// 편의를 위해 +=, + 연산자만 정의함.
inline Position operator +(const Position& left, const Position& right) {
	return Position(left.X + right.X, left.Y + right.Y);
}