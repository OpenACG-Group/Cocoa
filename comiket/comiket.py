import curses
from time import sleep


def calc_content_final_pos(win_w, x, y, content) -> tuple[int, int]:
    for ch in content:
        if ch.isascii():
            x += 1
        else:
            x += 2
        if x >= win_w:
            x = x % win_w
            y += 1
    return x, y


class Printer:
    def __init__(self, win_w, win_h, win) -> None:
        self.current_text_x = 0
        self.current_text_y = 0
        self.win_w = win_w
        self.win_h = win_h
        self.win = win

    def turn_next_page(self):
        self.current_text_x = 0
        self.current_text_y = 0
        self.win.clear()
        self.win.refresh()

    def put_paragraph(self, content):
        x = self.current_text_x
        y = self.current_text_y
        peek_x, peek_y = calc_content_final_pos(self.win_w, x, y, content)
        if peek_y >= self.win_h:
            self.turn_next_page()

        self.win.move(y, x)
        for ch in content:
            self.win.addch(ch)
            self.win.refresh()
            sleep(0.05)

        self.current_text_x = peek_x
        self.current_text_y = peek_y

    def put_newline(self):
        self.current_text_x = 0
        self.current_text_y += 1
        if self.current_text_y >= self.win_h:
            self.turn_next_page()
        self.win.move(self.current_text_y, self.current_text_x)
        self.win.refresh()


def launch_entry_point(stdscr):
    stdscr.clear()
    curses.cbreak()
    curses.noecho()
    stdscr.refresh()
    scr_height = curses.LINES
    scr_width = curses.COLS
    text_win_w = scr_width
    text_win_h = scr_height - 2

    ctl_win = curses.newwin(1, scr_width, 0, 0)
    text_win = curses.newwin(text_win_h, text_win_w, 2, 0)
    stdscr.move(1, 0)
    stdscr.hline('-', scr_width)
    stdscr.move(0, 0)

    ctl_win.move(0, 0)
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLUE)
    ctl_win.addstr('Cocoa Comiket Tiny Novel Engine', curses.color_pair(1) | curses.A_BOLD)
    ctl_win.refresh()

    printer = Printer(text_win_w, text_win_h, text_win)

    scenario = [
        "Hello, World! Just a simple test for this stupid (visual) novel engine.",
        "#NL",
        "这是标准 Unicode 中文测试，可以正常显示吗？",
        "#NP",
        "Next page!",
        "#NL"
    ]

    shall_exit = False
    # Comiket main loop
    stdscr.refresh()
    for line in scenario:
        if line == "#NL":
            printer.put_newline()
            printer.put_newline()
        elif line == "#NP":
            printer.turn_next_page()
        else:
            printer.put_paragraph(line)
        stdscr.getch()


# Start comiket framework
curses.wrapper(launch_entry_point)
