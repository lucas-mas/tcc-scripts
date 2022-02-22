/*
 * Implementa estimador através de janela deslizante.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define CONT_FRAC_STEPS     20

double lbeta(double a, double b) {

    return(lgamma(a) + lgamma(b) - lgamma(a + b));
}

double IB(double x, double a, double b) {

    double multiplier;
    double fraction = x/4.0;
    double d;
    int m, i;

    if (x >= (a + 1) / (a + b + 1)) return(1 - IB(1 - x, b, a));

    multiplier = a * log(x) + b * log(1-x) - log(a) - lbeta(a, b);
//printf("multiplier = %.4f, lbeta(%1.0f,%1.0f) = %e\n", multiplier, a, b, lbeta(a, b));
    for (i = CONT_FRAC_STEPS; i > 0; i--) {

        m = i >> 1;
        if (i & 1) {

            d = -(a + m)*(a + b + m)*x / ((a + 2*m) * (a + 2*m + 1));
        }
        else {

            d = m * (b - m) * x / ((a + 2*m) * (a + 2*m - 1));
        }

        fraction = d / fraction + 1;
    }

    fraction = 1 / fraction;
//printf("fraction = %.4f (%e)\n", fraction, fraction);
    return(exp(multiplier + log(fraction)));
}

double Fbin(int k, int n, double p) {

    return(1 - IB(p, (double) k + 1, (double) n - k));
}

double Pbin(int k, int n, double p) {

    return(exp(lgamma(n + 1) - lgamma(k + 1) - lgamma(n - k + 1) + k * log(p) + (n - k) * log(1 - p)));
}


/*
 * Find lowest value of k s.t. Fbin(k, n, p) >= q.
 * Binary search.
 */
int invFbin(int n, double p, double q) {

    int k;
    int lowerK = -1;
    int lowerBound, upperBound;
    double FbinAtK;
    double FbinAtLowerK;

    if (q <= 0.0) return(0);
    if (q >= 1.0) return(n);

    /*
     * Initial guess.
     */
    lowerBound = 0;
    upperBound = n;
    if (q < 0.5) upperBound = (int) (n * p);
    else lowerBound = (int) (n * p);


    k = (upperBound + lowerBound) >> 1;

//printf("k inicial = %d\n", k);    
    while(lowerBound < upperBound) {

        if (k == lowerK) {

            FbinAtK = FbinAtLowerK;
        }
        else {

            FbinAtK = Fbin(k, n, p);
        }

//printf("k = %d, lowerBound = %d, upperBound = %d, FbinAtK = %e\n", k, lowerBound, upperBound, FbinAtK);

        if (FbinAtK > q) {

            if (k == 0) return(0);

            lowerK = k - 1;
            FbinAtLowerK = Fbin(lowerK, n, p);
            if (FbinAtLowerK < q) return(k);

            upperBound = k - 1;
        }
        else {

            lowerBound = k + 1;
        }

        
        k = (lowerBound + upperBound) >> 1;
    }

//printf("k = %d, lowerBound = %d, upperBound = %d, FbinAtK = %e (exit)\n", k, lowerBound, upperBound, FbinAtK);

    return(k);
}

typedef struct {

    unsigned char * probes;
    int first;
    int last;
    int length;
    int maxLength;
    int totalOn;
    int totalOff;
} t_window;

t_window * window_init(int maxLength) {

    t_window * w;

    w = (t_window *) malloc(sizeof(t_window));
    w->probes = (unsigned char *) malloc(maxLength);
    w->length = 0;
    w->first = 0;
    w->last = 0;
    w->maxLength = maxLength;

    return(w);
}

void window_newprobe(t_window * w, unsigned char value) {

    if (w->length < w->maxLength) {

        w->probes[w->last] = value;
        w->length++;
    }
    else {

        if (w->probes[w->first] == 0) w->totalOff--;
        else w->totalOn--;

        w->probes[w->last] = value;
        w->first = (w->first + 1) % w->maxLength;
    }

    if (value == 0) w->totalOff++;
    else w->totalOn++;

    w->last = (w->last + 1) % w->maxLength;
}

void window_empty(t_window * w) {

    w->length = 0;
    w->first = 0;
    w->last = 0;
}

int window_getOn(t_window * w) {

    return(w->totalOn);
}

int window_getLength(t_window * w) {

    return(w->length);
}

unsigned char probes_next() {

    unsigned int input;
    unsigned char probe;

    if (feof(stdin)) return(0xFF);
    fscanf(stdin, "%d\n", & input);
    probe = 0xFF & input;

    return(probe);
}

int main(int argc, char ** argv) {

    int n, k;
//    double p, q, alpha;
    t_window * window;
    unsigned char probe;

    n = atoi(argv[1]);
//    alpha = (double) atof(argv[2]);

    window = window_init(n);
    while((probe = probes_next()) != 0xFF) {

        window_newprobe(window, probe);
        k = window_getOn(window);
//        n = window_getLength(window);
        printf("%.8f\n", (double) k / (double) n);
    }

    return(0);
}

