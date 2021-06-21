/*
    1-dimension cellular automaton
    NaSch Model Simulation
    By CTHua 2021/05/11
*/
#include "stdc++.h"
#include <curses.h>
#include <unistd.h>
#define StartX 1
#define StartY 1
WINDOW *win;
int speed = 1;
double averageSpeed = 0;
/* Initialize Random Number Generator */

// random_device
std::random_device rd;

// random_generator(use mersenne_twister_engine)
std::mt19937_64 generator(rd());

// random_distribution(uniform_distribution)
std::uniform_real_distribution<double> rand_double(0.0, 1.0);
std::uniform_int_distribution<int> rand_int(0, INT_MAX);
int rint() { return rand_int(generator); }
double rdouble() { return rand_double(generator); }

/**/
class Car
{
public:
    //Default constructor for Car, it means this is an empty car.
    Car()
    {
        velocity = -1;
        distance = -1;
    }

    //Constructor for Car, it means this is a Car, velocity is v;
    Car(int v)
    {
        velocity = v;
    }

    //Step 1, if velocity < vmax, then velocity + 1
    void faster(int vmax_)
    {
        velocity = std::min(vmax_, velocity + 1);
    }

    //Step 2, if velocity > distance between the front car, velocity = distance
    void slower()
    {
        velocity = velocity > distance ? distance : velocity;
    }

    //Step 3, if velocity > 0, then velocity has a probability "prob" let the velocity - 1.
    void random_slower(double prob)
    {
        double r = rdouble();
        if (r < prob && velocity > 0)
            velocity = velocity - 1;
    }

    //do Step 1, 2, 3
    void oneRound(int vmax, double prob)
    {
        if (velocity == -1)
            return;
        faster(vmax);
        slower();
        random_slower(prob);
    }

    /*Some functions let outer scope can get prototype of this class */
    int get_velocity() { return velocity; }
    int get_distance() { return distance; }
    void set_velocity(int velocity_) { velocity = velocity_; }
    void set_distance(int distance_) { distance = distance_; }

private:
    int velocity;
    int distance;
};

class Lane
{
public:
    //Constructor of one lane of high way
    Lane(int length_, int carNum_, int vmax_, double slowProb_, int laneNum_) : length(length_), vmax(vmax_), carNum(carNum_), laneNum(laneNum_), slowProb(slowProb_)
    {
        list = new Car[length];
        int *carPos = new int[carNum];
        for (int i = 0; i < carNum; i++)
        {
            int randomPos = rint() % length;
            bool duplicate = true;
            while (duplicate)
            {
                int countDuplicate = 0;
                for (countDuplicate = 0; countDuplicate < i; countDuplicate++)
                {
                    if (randomPos == carPos[countDuplicate])
                    {
                        randomPos = rint() % length;
                        break;
                    }
                }
                if (countDuplicate == i)
                    duplicate = false;
            }
            carPos[i] = randomPos;
            list[carPos[i]] = Car(rint() % vmax + 1);
        }
        for (int i = 0; i < carNum_; i++)
            update_distance();
        puts("origin statement");
        debugPrint();
        puts("-------");
        delete[] carPos;
    }
        ~Lane()
    {
        delete[] list;
    }
    void update_distance()
    {
        int calculatedCount = 0;
        for (int i = 0; i < length && calculatedCount < carNum; i++)
        {
            while (list[i].get_velocity() == -1)
                i++;
            for (int j = i + 1; j <= length + i; j++)
            {
                int pos = j % length;
                if (list[pos].get_velocity() != -1)
                {
                    int distance = j - i;
                    list[i].set_distance(distance - 1);
                    i = pos - 1;
                    calculatedCount++;
                    break;
                }
            }
        }
    }
    void nextPos()
    {
        for (int i = 0; i < length; i++)
            list[i].oneRound(vmax, slowProb);
        for (int i = 0; i < length; i++)
        {
            if (list[i].get_velocity() != -1 && list[i].get_distance() != -1)
            {
                int newPos = (list[i].get_velocity() + i) % length;
                list[newPos] = Car(list[i].get_velocity());
                list[newPos].set_distance(-1);
                if (newPos != i)
                    list[i].set_velocity(-1);
            }
        }
    }
    void debugPrint()
    {
        for (int i = 0; i < length; i++)
        {
            if (list[i].get_velocity() != -1)
                printf("%d(%d) ", list[i].get_velocity(), list[i].get_distance());
            else
                printf("  .  ");
        }
        puts("");
    }
    void colorPrint()
    {
        for (int i = 0; i < length; i++)
        {
            int vel = list[i].get_velocity();
            if (vel != -1)
            {
                averageSpeed += vel;
                wattron(win, COLOR_PAIR(vel + 1));
                mvwprintw(win, StartY, StartX + i * 2, "  ");
                wattroff(win, COLOR_PAIR(vel + 1));
                usleep(1000 / (vel + 1));
            }
            else
            {
                wattron(win, COLOR_PAIR(vmax + 2));
                mvwprintw(win, StartY, StartX + i * 2, "  ");
                wattroff(win, COLOR_PAIR(vmax + 2));
            }
            wrefresh(win);
        }
    }

private:
    Car *list;
    int length;
    int carNum;
    int laneNum;
    int vmax;
    double slowProb;
};
int main(int argc, char **argv)
{
    if (argc != 7)
    {
        std::cerr << "Usage: " << std::string(argv[0]) << " length number_of_car max_velocity probability round speed" << std::endl;
        std::cerr << "---" << std::endl;        
        std::cerr << "length: length of lane" << std::endl;
        std::cerr << "number_of_car: number_of_car in one lane" << std::endl;
        std::cerr << "max_velocity: maximum velocity in one lane" << std::endl;
        std::cerr << "probability: probability let car be slower" << std::endl;
        std::cerr << "round: simulate round" << std::endl;
        std::cerr << "speed: simulate speed" << std::endl;
        exit(1);
    }

    /* Initialize Properties*/
    // length of highway
    int L = atoi(argv[1]);

    // number of Car
    int numberOfCar = atoi(argv[2]);

    // the maximum velocity of cars
    int vmax = atoi(argv[3]);

    // probaility let car slower
    double probability = atof(argv[4]);

    // simulate times
    int round = atoi(argv[5]);

    speed = atoi(argv[6]);
    /*Check Properties*/
    if (L <= 1)
    {
        std::cerr << "Length must be larger than 1" << std::endl;
        return 1;
    }
    if (numberOfCar >= L)
    {
        std::cerr << "number_of_car must be smaller than length" << std::endl;
        return 1;
    }
    if (vmax <= 0)
    {
        std::cerr << "max_velocity must be larger than 0" << std::endl;
        return 1;
    }
    if (probability >= 1)
    {
        std::cerr << "probability must be smaller than 1" << std::endl;
        return 1;
    }
    if (round <= 0)
    {
        std::cerr << "round must be larger than 0" << std::endl;
        return 1;
    }
    if (speed <= 0)
    {
        std::cerr << "speed must be larger than 0" << std::endl;
        return 1;
    }

    /* Start Simulation */
    //Initialize the random car position
    int *random_pos = new int[L];
    for (int i = 0; i < L; i++)
        random_pos[i] = i;
    for (int i = 0; i < (2 << 20); i++)
    {
        int x = rint() % L, y = rint() % L;
        int tmp = random_pos[x];
        random_pos[x] = random_pos[y];
        random_pos[y] = tmp;
    }

    //Start simulate
    initscr();
    start_color();
    init_color(vmax + 2, 780, 780, 780);
    init_pair(vmax + 2, vmax + 2, vmax + 2);
    curs_set(0);
    for (int i = 0; i <= vmax; i++)
    {
        int red = 1000 / (vmax + 1) * (vmax - i);
        int green = 1000 / (vmax + 1) * (i + 1);
        init_color(i + 50, red, green, 0);
        init_pair(i + 1, vmax + 1, i + 50);
    }

    Lane highway(L, numberOfCar, vmax, probability, 1);
    clear();
    win = newwin(8, L * 2 + 2, 0, 0);
    box(win, '|', '-');
    for (int r = 1; r <= round; r++)
    {
        highway.nextPos();
        highway.update_distance();
        //highway.debugPrint();
        highway.colorPrint();

        mvwprintw(win, 4, 1, "Speed: %dx", speed);
        mvwprintw(win, 5, 1, "Now round: %d", r);
        mvwprintw(win, 6, 1, "Average Speed: %f", averageSpeed / numberOfCar);
        touchwin(win);
        wrefresh(win);
        usleep(500000 / speed);
        averageSpeed = 0;
    }
    getchar();
    endwin();
    return 0;
}