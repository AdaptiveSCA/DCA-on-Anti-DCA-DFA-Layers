#include "ENCA.h"
void generate_map(uint8_t map[16])
{
    Aff4 ed, ed_inv;
    genaffinepairM4(&ed, &ed_inv);
    for(int i = 0; i < 16; i++)
    {
        map[i] = affineU4(ed, Sbox[i ^ real_key]);
    }
}

void DCA(uint8_t map[16])
{
    uint8_t kk, state;
    int trace[16][4] = {0};
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(map[i] & idM4[j]) trace[i][j] = 1;
            else trace[i][j] = 0;
        }
    }
    ////
    double keystate[16][4][5] = {0.0};
    double keycurve[16][5] = {0.0};
    for(int j = 0; j < 16; j++)//key
    {
        kk = j;
        for(int r = 0; r < 4; r++) // bit
        {
            double split0[4] = {0.0}; //----------
            double split1[4] = {0.0};
            int count0 = 0;
            int count1 = 0;
            for(int i = 0; i < 16; i++) // plaintext
            {
                state = Sbox[i ^ kk];
                if(state & idM4[r])
                {
                    for(int g = 0; g < 4; g++)
                    {
                        split1[g] += trace[i][g];
                    }
                    count1++;
                }
                else
                {
                    for(int g = 0; g < 4; g++)
                    {
                        split0[g] += trace[i][g];
                    }
                    count0++;
                }
            }
            for(int y = 0; y < 4; y++)
            {
                split0[y] = split0[y] / count0;
                split1[y] = split1[y] / count1;
            }
            for(int y = 0; y < 4; y++)
            {
                if(split0[y] > split1[y]) keystate[kk][r][y] = split0[y] - split1[y];
                else keystate[kk][r][y] = split1[y] - split0[y];
            }
        }
        int maxord = 0;
        double maxvalue = 0.0;
        for(int i = 0; i < 4; i++)
        {
            double peak = 0.0;
            for(int j = 0; j < 4; j++)
            {
                if (keystate[kk][i][j] > peak) peak = keystate[kk][i][j]; // max in each curve
            }
            keystate[kk][i][4] = peak;
        }
        for(int i = 0; i < 4; i++)
        {
            if(keystate[kk][i][4] > maxvalue) 
            {
                maxord = i; // the corresponding bit of the peak
                maxvalue = keystate[kk][i][4];
            }
        }
        for(int i = 0; i < 5; i++) // the peak vaule as the peak of the key
        {
            keycurve[kk][i] = keystate[kk][maxord][i];
        }
    }
    // double key_guess_value = 0.0;
    // int key_guess = 0;
    for(int k = 0; k < 16; k++)
    {

        printf("key %d: %f\n", k, keycurve[k][4]);
    }
    // if(key_guess == real_key) return 1;
    // return 0;
}

void IDCA(uint8_t map[16])
{
    uint8_t  kk, state;
    int trace[16][4] = {0};
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(map[i] & idM4[j]) trace[i][j] = 1;
            else trace[i][j] = 0;
        }
    }
    ////
    double keycurve[16][5] = {0.0};
    for(int kk = 0; kk < 16; kk++) //key
    {
        double lecurve[16][5] = {0.0};
        for(int le = 0; le < 16; le++) // linear encoding
        {
            double split0[4] = {0.0};
            double split1[4] = {0.0};
            int count0 = 0;
            int count1 = 0;
            for(int i = 0; i < 16; i++) // plaintext
            {
                state = xor[le & Sbox[i ^ kk]];
                if(state)
                {
                    for(int g = 0; g < 4; g++)
                    {
                        split1[g] += trace[i][g];
                    }
                    count1++;
                }
                else
                {
                    for(int g = 0; g < 4; g++)
                    {
                        split0[g] += trace[i][g];
                    }
                    count0++;
                }
            }
            for(int y = 0; y < 4; y++)
            {
                split0[y] = split0[y] / count0;
                split1[y] = split1[y] / count1;
            }
            for(int y = 0; y < 4; y++)
            {
                if(split0[y] >= split1[y]) lecurve[le][y] = split0[y] - split1[y];
                else lecurve[le][y] = split1[y] - split0[y];
            }
            double peak = 0.0;
            for(int j = 0; j < 4; j++)
            {
                if (lecurve[le][j] > peak) peak = lecurve[le][j]; // max in each curve
            }
            lecurve[le][4] = peak;
        }
        double maxlepeak = 0.0;
        uint8_t maxle = 0;
        for(int le = 0; le < 16; le++) // linear encoding
        {
            if(lecurve[le][4] > maxlepeak) 
            {
                maxle = le;
                maxlepeak = lecurve[le][4];
            }
        }
        if(maxlepeak > 0.30) 
        {
            for(int i = 0; i < 5; i++)
            {
                keycurve[kk][i] = lecurve[maxle][i];
            }
        }
        else if(maxlepeak >= 0.20 && maxlepeak <= 0.30) 
        {
            for(int le = 0; le < 16; le++) // look for a value 0.25
            {
                if(lecurve[le][8] == 0.25) 
                {
                    for(int i = 0; i < 5; i++)
                    {
                        keycurve[kk][i] = lecurve[le][i];
                    }
                    break;
                }
            }
        }
        else if(maxlepeak < 0.20) 
        {
            for(int le = 0; le < 16; le++) // look for a value 0.25
            {
                if(lecurve[le][8] == 0.0) 
                {
                    for(int i = 0; i < 5; i++)
                    {
                        keycurve[kk][i] = lecurve[le][i];
                    }
                    break;
                }
            }
        }
    }
    double maxkeypeak = 0.0;
    uint8_t maxkey = 0;
    for(int i = 0; i < 16; i++) // ranking the candidates
    {
        printf("key %d: %f\n", i, keycurve[i][4]);
    }
}

void CPA(uint8_t map[16])
{
    int trace[16][4] = {0};
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(map[i] & idM4[j]) trace[i][j] = 1;
            else trace[i][j] = 0;
        }
    }
    ////
    double score[4][4] = {0.0};
    double kmax[16] = {0.0};
    int f = 0, g = 0, k, j, i, c;
    uint8_t key_candidate[4] = {0};
    double key_candidate_value[4] = {0.0};
    
    for(k = 0; k < 16; k++) // key
    {
        for(c = 0; c < 4; c++) // bits of SBox
        {    
            for(j = 0; j < 4; j++) // samples of trace
            {
                int Nf0 = 0, Nf1 = 0, Ng0 = 0, Ng1 = 0, N00 = 0, N01 = 0, N10 = 0, N11 = 0;
                for(i = 0; i < 16; i++) // plaintext
                {
                    f = trace[i][j];
                    if(f) Nf1++;
                    else Nf0++;

                    g = (Sbox[i ^ k] >> (3 - c)) & 1; // a bit of SBox
                    if(g) Ng1++;
                    else Ng0++;
                    
                    if(f == 1 && g == 1) N11++;
                    else if(f == 0 & g == 0) N00++;
                    else if(f == 1 & g == 0) N10++;
                    else N01++;
                }
                score[c][j] = abs((N11 * N00 - N10 * N01)) * 1.0 / (sqrt(Nf1) * sqrt(Nf0) * sqrt(Ng1) * sqrt(Ng0));
            }
        }
        double max = 0.0;
        for(c = 0; c < 4; c++)
        { 
            for(j = 0; j < 4; j++)
            {
                if(score[c][j] > max) max = score[c][j];
            }
        }
        printf("key %d: %f\n", k, max);
    }
}

void CA(uint8_t map[16])
{
    uint8_t temp;
    uint8_t trace[120][8] = {0};
    int num = 0;
    for(int i = 0; i < 15; i++)
    {
        for(int j = i + 1; j < 16; j++)
        {
            temp = map[i] ^ map[j];
            for(int r = 0; r < 4; r++)
            {
                if(temp & idM4[r]) trace[num][r] = 1;
                else trace[num][r] = 0;
            }
            num++;
        }
    }
    ////
    double score[4][4] = {0.0};
    double kmax[16] = {0.0};
    int f = 0, g = 0, k, j, x1, x2, c;
    uint8_t key_candidate[4] = {0};
    double key_candidate_value[4] = {0.0};
    
    for(k = 0; k < 16; k++) // key
    {
        for(c = 0; c < 4; c++) // bits of SBox
        {       
            for(j = 0; j < 4; j++) // column
            {
                int Nf0 = 0, Nf1 = 0, Ng0 = 0, Ng1 = 0, N00 = 0, N01 = 0, N10 = 0, N11 = 0;
                num = 0;
                for(x1 = 0; x1 < 15; x1++) // plaintext
                {
                    for(x2 = x1 + 1; x2 < 16; x2++) // plaintext
                    {
                        f = trace[num++][j];
                        if(f) Nf1++;
                        else Nf0++;

                        g = ((Sbox[x1 ^ k] ^ Sbox[x2 ^ k]) >> (3 - c)) & 1; // a bit of SBox
                        if(g) Ng1++;
                        else Ng0++;
                        
                        if(f == 1 && g == 1) N11++;
                        else if(f == 0 & g == 0) N00++;
                        else if(f == 1 & g == 0) N10++;
                        else N01++;
                    }
                }
                score[c][j] = abs((N11 * N00 - N10 * N01)) * 1.0 / (sqrt(Nf1) * sqrt(Nf0) * sqrt(Ng1) * sqrt(Ng0));
            }
        }
        double max = 0.0;
        for(c = 0; c < 4; c++)
        { 
            for(j = 0; j < 4; j++)
            {
                if(score[c][j] > max) max = score[c][j];
            }
        }
        printf("key %d: %f\n", k, max);
    }
    
}

void SA(uint8_t map[16])
{
    uint8_t inv_Sbox[16];
    for(int i = 0; i < 16; i++)
    {
        inv_Sbox[Sbox[i]] = i; // inverse SBox
    }
    int trace[16][4] = {0};
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(map[i] & idM4[j]) trace[i][j] = 1;
            else trace[i][j] = 0;
        }
    }
    ////
    int kmax[16] = {0};
    int k, j, x, w, WT;
    for(k = 0; k < 16; k++) // key
    {
        for(w = 0; w < 16; w++) // w <- Walsh transforms
        {
            for(j = 0; j < 4; j++) // column
            {
                WT = 0;
                for(x = 0; x < 16; x++) // plaintext
                {
                    if(trace[inv_Sbox[x] ^ k][j] == xor[x & w]) WT++;
                    else WT--;
                }
                kmax[k] += abs(WT);
            }
        }
    }
    for(k = 0; k < 16; k++)
    {
        printf("key %d: %d\n", k, kmax[k]);
    }
}

int main()
{
    uint8_t map[16];
    generate_map(map);
    printf("-----DCA-----\n");
    DCA(map);
    printf("-----IDCA-----\n");
    IDCA(map);
    printf("-----CPA-----\n");
    CPA(map);
    printf("-----CA-----\n");
    CA(map);
    printf("-----SA-----\n");
    SA(map);
    return 0;
}