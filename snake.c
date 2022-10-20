#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { key_escape = 27 };
enum { delay_duration = 100 };

struct star {
    int cur_x, cur_y;
    int dx, dy;
};

struct list_stars {
    struct star star;
    struct list_stars *next;
    struct list_stars *previos;
};

struct list_stars *node_create(struct star s)
{
    struct list_stars *p_list = malloc(sizeof(struct list_stars));
    p_list->star = s;
    p_list->next = NULL;
    p_list->previos = NULL;
    return p_list;
}

void add_end(struct list_stars **end)
{
    struct star new_star = (struct star) { (*end)->star.cur_x - 1, (*end)->star.cur_y, 0, 0 };
    struct list_stars *new_star_in_list = node_create(new_star);
    new_star_in_list->previos = *end;
    (*end)->next = new_star_in_list;
    (*end) = new_star_in_list;
}

struct purpose {
    int cur_x, cur_y;
};


static void show_star(const struct star *s)
{
    move(s->cur_y, s->cur_x);
    addch('*');
}

void show_list(const struct list_stars *plist)
{
    struct list_stars *tmp = (struct list_stars *)plist;
    while (tmp) {
        show_star(&(tmp->star));
        tmp = tmp->next;
    }
}

static void show_purpose(const struct purpose *p)
{
    move(p->cur_y, p->cur_x);
    addch('#');
    refresh();
}

static void hide_purpose(const struct purpose *p)
{
    move(p->cur_y, p->cur_x);
    addch('*');
    refresh();
}

static void hide_star(const struct star *s)
{
    move(s->cur_y, s->cur_x);
    addch(' ');
}

static void check(int *coord, int max)
{
    if (*coord < 0)
        *coord += max;
    else
    if (*coord > max)
        *coord -= max;
}

/*
static void move_star(struct star *s, int max_x, int max_y)
{
    hide_star(s);
    s->cur_x += s->dx;
    check(&s->cur_x, max_x);
    s->cur_y += s->dy;
    check(&s->cur_y, max_y);
    show_star(s);
}
*/

static void move_list_stars(struct list_stars **begin, struct list_stars **end, int max_x, int max_y)
{
    hide_star(&((*end)->star));

    if (*begin != *end) {
        (*begin)->previos = *end;
        *end = (*end)->previos;
        (*end)->next = NULL;
        (*begin)->previos->next = *begin;
        *begin = (*begin)->previos;
        (*begin)->previos = NULL;

        (*begin)->star.dx = (*begin)->next->star.dx;
        (*begin)->star.dy = (*begin)->next->star.dy;

        (*begin)->star.cur_x = (*begin)->next->star.cur_x + (*begin)->next->star.dx;
        check(&((*begin)->star.cur_x), max_x);
        (*begin)->star.cur_y = (*begin)->next->star.cur_y + (*begin)->next->star.dy;
        check(&((*begin)->star.cur_y), max_y);
    } else {
        (*begin)->star.cur_x += (*begin)->star.dx;
        (*begin)->star.cur_y += (*begin)->star.dy;
    }

    show_star(&((*begin)->star));
    refresh();
}

static void set_direction(struct star *s, int dx, int dy)
{
    s->dx = dx;
    s->dy = dy;
}

static void set_direction_list(struct list_stars *begin, int dx, int dy)
{
    begin->star.dx = dx;
    begin->star.dy = dy;
}

static void handle_resize(struct star *s, int *col, int *row)
{
    getmaxyx(stdscr, *row, *col);
    if (s->cur_x > *col)
        s->cur_x = *col;
    if (s->cur_y > *row)
        s->cur_y = *row;
}

static void new_purpose(struct purpose *p, int max_x, int max_y)
{
    long x = rand();
    p->cur_x = (int)(max_x*x/(RAND_MAX+1.0));
    long y = rand();
    p->cur_y = (int)(max_y*y/(RAND_MAX+1.0));
}

int main()
{
    int rtime = time(NULL);
    srand(rtime);

    int row, col, key;
    struct star s;
    struct purpose p;
    initscr();
    cbreak();
	int speed = delay_duration;
    timeout(speed);
    keypad(stdscr, 1);
    noecho();
    curs_set(0);
    getmaxyx(stdscr, row, col);

    s.cur_x = col/2;
    s.cur_y = row/2;
    set_direction(&s, -1, 0);
    struct list_stars *begin = node_create(s);
    struct list_stars *end = begin;
//    add_end(&end);
//    add_end(&end);

    new_purpose(&p, col, row);
    show_purpose(&p);

    show_list(begin);
    while ((key = getch()) != key_escape) {
        switch (key) {
            case ' ':
                set_direction_list(begin, 0, 0);
                break;
            case KEY_UP:
                set_direction_list(begin, 0, -1);
				timeout(speed * 1.3);
                break;
            case KEY_DOWN:
                set_direction_list(begin, 0, 1);
				timeout(speed * 1.3);
                break;
            case KEY_LEFT:
                set_direction_list(begin, -1, 0);
				timeout(speed);
                break;
            case KEY_RIGHT:
                set_direction_list(begin, 1, 0);
				timeout(speed);
                break;
            case ERR:
                //move_star(&s, col-1, row-1);
                move_list_stars(&begin, &end, col-1, row-1);
                show_purpose(&p);
                break;
            case KEY_RESIZE:
                handle_resize(&s, &col, &row);
        }
        if (begin->star.cur_x == p.cur_x && begin->star.cur_y == p.cur_y) {
			hide_purpose(&p);
            new_purpose(&p, col, row);
            show_purpose(&p);
            add_end(&end);
			speed -= speed / 10;
			if (begin->star.dy != 0) {
				timeout(speed * 1.3);
			} else {
				timeout(speed);
			}
        }
    }
    endwin();
    return 0;
}
