#include "ListViewEx.h"

constexpr int ROW_SPACE = 3;

ListViewEx::ListViewEx(HWND hWnd, HMENU id) : rowWidth(0) {
	this->hWnd = CreateWindow(
		WC_ENTRYLISTVIEW, NULL, 
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hWnd, id, NULL, this);

	hoverBrush = CreateSolidBrush(RGB(220, 220, 220));
	colorBrush = CreateSolidBrush(RGB(126, 229, 162));
}

void ListViewEx::Register() {
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = ListViewEx::WndProc_;
	wc.lpszClassName = WC_ENTRYLISTVIEW;
	wc.cbWndExtra = sizeof(ListViewEx*);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);
}

void ListViewEx::AddColumn(const ListViewExColumn& column) {
	columns.push_back(column);
	rowWidth += column.width;
}

void ListViewEx::AddItem(const std::any& value) {
	items.push_back(value);
}

void ListViewEx::DeleteAllItems() {
	items.clear();
}

void ListViewEx::Move(int x, int y, int width, int height, bool repaint) {
	MoveWindow(hWnd, x, y, width, height, repaint);
}

LRESULT CALLBACK ListViewEx::WndProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_NCCREATE) {
		LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		auto lv = reinterpret_cast<ListViewEx*>(pcs->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(lv));
		return TRUE;
	}

	auto lv = reinterpret_cast<ListViewEx*>(GetWindowLongPtr(hWnd, 0));
	return lv->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ListViewEx::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
	HANDLE_MSG(hwnd, WM_VSCROLL, OnVScroll);
	HANDLE_MSG(hwnd, WM_HSCROLL, OnHScroll);
	HANDLE_MSG(hwnd, WM_MOUSEWHEEL, OnMouseWheel);
	HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMouseMove);
	HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
	HANDLE_MSG(hwnd, WM_SETFONT, OnSetFont);
	HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	case WM_MOUSEHWHEEL:
		OnMouseHWheel(hwnd, HIWORD(wParam));
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

BOOL ListViewEx::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) {
	// SCROLLINFO を初期化
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	si.nPos = 0;
	si.nMin = 0;
	SetScrollInfo(hWnd, SB_VERT, &si, FALSE);

	SCROLLINFO hsi;
	hsi.cbSize = sizeof(hsi);
	hsi.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	hsi.nPos = 0;
	hsi.nMin = 0;
	SetScrollInfo(hWnd, SB_HORZ, &hsi, FALSE);

	return TRUE;
}

void ListViewEx::OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos) {
	// 現在の SCROLLINFO を取得
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &si);

	// スクロール量
	int dy = 0;

	switch (code) {
	case SB_LINEUP:
		dy = -rowHeight;
		break;
	case SB_LINEDOWN:
		dy = rowHeight;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		dy = pos - si.nPos;
		break;
	case SB_PAGEDOWN:
		dy = -static_cast<int>(si.nPage);
		break;
	case SB_PAGEUP:
		dy = si.nPage;
		break;
	default:
		dy = 0;
		break;
	}

	SetScrollPos(hWnd, SB_VERT, si.nPos + dy, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos) {
	// 現在の SCROLLINFO を取得
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_HORZ, &si);

	// スクロール量
	int dx = 0;

	switch (code) {
	case SB_LINELEFT:
		dx = -10;
		break;
	case SB_LINERIGHT:
		dx = 10;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		dx = pos - si.nPos;
		break;
	case SB_PAGELEFT:
		dx = -static_cast<int>(si.nPage);
		break;
	case SB_PAGERIGHT:
		dx = si.nPage;
		break;
	default:
		dx = 0;
		break;
	}

	SetScrollPos(hWnd, SB_HORZ, si.nPos + dx, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys) {
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, SB_VERT, &si);

	int dy = 0;
	dy = zDelta / 120 * -(rowHeight * 2);

	SetScrollPos(hWnd, SB_VERT, si.nPos + dy, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnMouseHWheel(HWND hWnd, int delta) {
	// TODO: マウスを左に回転させると delta が 65146 になる
	/*SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, SB_HORZ, &si);

	int dx = 0;
	dx = delta / 120 * 20;

	SetScrollPos(hWnd, SB_HORZ, si.nPos + dx, TRUE);
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);*/
}

void ListViewEx::OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags) {
	// 再描画
	InvalidateRect(hWnd, NULL, TRUE);
}

void ListViewEx::OnLButtonDown(HWND hWnd, BOOL doubleClick, int x, int y, UINT keyFlags) {
	if (!doubleClick) {
		if (x < rowWidth) {
			// 垂直スクロールバーを取得
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(hWnd, SB_VERT, &si);

			int index = (y + si.nPos) / rowHeight;
			selection.start = index;
			selection.end = index;
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}
}

void ListViewEx::OnSetFont(HWND hWndCtl, HFONT hFont, BOOL fRedraw) {
	font = hFont;
	// フォントの高さを取得
	auto hdc = GetDC(hWnd);
	SelectObject(hdc, font);
	TEXTMETRIC metrics;
	GetTextMetrics(hdc, &metrics);
	// 行の高さを計算
	rowHeight = metrics.tmHeight + ROW_SPACE;
}

void ListViewEx::OnPaint(HWND hWnd) {
	// 画面のサイズを取得
	RECT rect;
	GetClientRect(hWnd, &rect);

	// 垂直スクロールバー
	SCROLLINFO verticalScrollInfo;
	verticalScrollInfo.cbSize = sizeof(verticalScrollInfo);
	verticalScrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &verticalScrollInfo);

	// 平行スクロールバー
	SCROLLINFO horizontalScrollInfo;
	horizontalScrollInfo.cbSize = sizeof(horizontalScrollInfo);
	horizontalScrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_HORZ, &horizontalScrollInfo);
	
	// カーソル位置を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(hWnd, &cursorPos);

	PAINTSTRUCT ps;
	HDC _hdc = BeginPaint(hWnd, &ps);
	HDC hdc = CreateCompatibleDC(_hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(_hdc, rect.right, rect.bottom);
	SelectObject(hdc, bitmap);

	FillRect(hdc, &rect, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

	// フォントを設定
	SelectObject(hdc, font);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));

	int count = 0;
	for (const auto& item : items) {
		// 描画する Y 座標
		auto y = rowHeight * count - verticalScrollInfo.nPos;
		// カーソルが重なっていたら背景を描画
		if (cursorPos.y >= y && cursorPos.y < y + rowHeight && cursorPos.x < rowWidth) {
			RECT background = { 0, y, rowWidth, y + rowHeight };
			FillRect(hdc, &background, hoverBrush);
		}

		// 選択されていたら背景を描画
		if (count >= selection.start && count <= selection.end) {
			RECT background = { 0, y, rowWidth, y + rowHeight };
			FillRect(hdc, &background, colorBrush);
		}

		// 描画する X 座標
		int x = 0;
		for (const auto& column : columns) {
			// 文字列を描画
			RECT textRect = { x - horizontalScrollInfo.nPos, y, x - horizontalScrollInfo.nPos + column.width, y + rowHeight };
			DrawText(hdc, column.get(item).c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER);

			x += column.width;
		}

		count++;
	}

	int maxY = rowHeight * count;

	BitBlt(_hdc, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);

	DeleteObject(bitmap);
	DeleteDC(hdc);

	EndPaint(hWnd, &ps);

	// 垂直スクロールバー
	verticalScrollInfo.nPage = rect.bottom;
	verticalScrollInfo.nMax = maxY;
	SetScrollInfo(hWnd, SB_VERT, &verticalScrollInfo, TRUE);

	// 平行スクロールバー
	horizontalScrollInfo.nPage = rect.right;
	horizontalScrollInfo.nMax = rowWidth;
	SetScrollInfo(hWnd, SB_HORZ, &horizontalScrollInfo, TRUE);
}
