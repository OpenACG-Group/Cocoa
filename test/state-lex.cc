#include <iostream>
using namespace std;

int main()
{
    int state = 0;
    char ch;
    while (cin >> ch)
    {
        int oldState = state;
        switch (state)
        {
        case 0:
            if (isdigit(ch))
                state = 1;
            break;
        case 1:
            if (isdigit(ch))
                state = 1;
            else if (ch == '.')
                state = 2;
            else if (ch == 'e')
                state = 4;
            else if (ch == ';')
                state = 8;
            break;
        case 2:
            if (isdigit(ch))
                state = 3;
            break;
        case 3:
            if (isdigit(ch))
                state = 3;
            else if (ch == 'e')
                state = 4;
            else if (ch == ';')
                state = 9;
            break;
        case 4:
            if (ch == '-' || ch == '+')
                state = 5;
            else if (isdigit(ch))
                state = 6;
            break;
        case 5:
            if (isdigit(ch))
                state = 6;
            break;
        case 6:
            if (isdigit(ch))
                state = 6;
            else if (ch == ';')
                state = 7;
            break;
        }

        cout << "S" << oldState << " -> " << state << endl;
        switch (state)
        {
        case 7:
            cout << "A float number with pointer" << endl;
            state = 0;
            break;
        case 8:
            cout << "An integer" << endl;
            state = 0;
            break;
        case 9:
            cout << "A float number" << endl;
            state = 0;
            break;
        }
    }
    
    return 0;
}
