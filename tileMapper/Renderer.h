#ifndef RENDERER_H
#define RENDERER_H

#pragma once

struct BitmapData{
	void* data;
	int width;
	int height;
};

inline int RGBA(int r, int g, int b, int a) {
	return (r << 24) | (g << 16) | (b << 8) | (a);
}

void DrawBox(BitmapData bitmap, int x, int y, int w, int h, int col);

void DrawBitmap(BitmapData bitmap, int x, int y, int w, int h, BitmapData sprite);

void DrawText(BitmapData bitmap, char* text, int size, int x, int y);

void Render(BitmapData frameBuffer);

BitmapData LoadBMPFile(char* fileName);

#endif