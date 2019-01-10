#pragma once

#include <vector>
#include <functional>
#include <any>
#include <tchar.h>
#include <Windows.h>
#include <windowsx.h>
#include <fmt/format.h>

#define WC_ENTRYLISTVIEW _T("EntryListView")

struct ListViewExColumn {
	int width;
	std::wstring header;
	std::function<std::wstring(std::any v)> get;
};

struct ListViewExSelection {
	int start;
	int end;
};

class ListViewEx {
public:
	ListViewExSelection selection;

	ListViewEx(HWND hWnd, HMENU id);

	static void Register();

	void AddColumn(const ListViewExColumn& column);
	void AddItem(const std::any& value);
	void DeleteAllItems();
	void Move(int x, int y, int width, int height, bool repaint);
private:
	HWND hWnd;

	HBRUSH hoverBrush;
	HBRUSH colorBrush;
	std::vector<std::any> items;
	std::vector<ListViewExColumn> columns;
	HFONT font;
	int rowHeight;

	static LRESULT CALLBACK WndProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	void OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos);
	void OnHScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos);
	void OnMouseWheel(HWND hWnd, int xPos, int yPos, int zDelta, UINT fwKeys);
	void OnMouseHWheel(HWND hWnd, int delta);
	void OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags);
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void OnSetFont(HWND hWndCtl, HFONT hfont, BOOL fRedraw);
	void OnPaint(HWND hWnd);
};
