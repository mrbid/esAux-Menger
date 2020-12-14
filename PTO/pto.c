/*
--------------------------------------------------
    James William Fletcher (james@voxdsp.com)
        December 2020 - pto.c
--------------------------------------------------
    ASCII PLY to C Header coverter for OpenGL Buffers.

    Created for the use in esAux renderer project;
    https://github.com/esaux

    https://en.wikipedia.org/wiki/PLY_(file_format)

    no use of mmap() to maintain portability.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wunused-result"

void appendFile(char source[], char destination[])
{
    FILE* f1 = fopen(source, "r");
    FILE* f2 = fopen(destination, "a");
 
    if(f1 && f2)
    {
        fprintf(f2, "\n");
        
        char buf[256];
        while(!feof(f1))
        {
            fgets(buf, sizeof(buf), f1);
            fprintf(f2, "%s", buf);
        }
    
        fclose(f1);
        fclose(f2);
    }
}

int truncLast(FILE* f)
{
    fseeko(f, -1, SEEK_END);
    ftruncate(fileno(f), ftello(f));
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Please specify an input file.\n");
        return 0;
    }

    char name[32] = {0};
    strcat(name, argv[1]);
    char* p = strstr(name, ".");
    *p = 0x00;

    FILE* f_vertex_array = fopen("vertex_array.txt", "w");
    fprintf(f_vertex_array, "const GLfloat %s_vertices[] = {", name);
    FILE* f_index_array  = fopen("index_array.txt", "w");
    fprintf(f_index_array, "const GLuint %s_indices[] = {", name);
    FILE* f_normal_array = fopen("normal_array.txt", "w");
    fprintf(f_normal_array, "const GLfloat %s_normals[] = {", name);
    FILE* f_color_array  = fopen("color_array.txt", "w");
    fprintf(f_color_array, "const GLfloat %s_colors[] = {", name);

    unsigned int numvert=0, numind=0, numcol=0, numnorm=0;

    int mode = 0;
    printf("Open: %s\n", argv[1]);
    FILE* f = fopen(argv[1], "r");
    if(f)
    {
        char line[256];
        while(fgets(line, 256, f) != NULL)
        {
            //printf("%s\n",line);
            if(strcmp(line, "end_header\n") == 0)
            {
                mode = 1;
                continue;
            }

            // load index
            if(mode == 2)
            {
                unsigned int n,x,y,z;
                if(sscanf(line, "%u %u %u %u", &n, &x, &y, &z) == 4)
                {
                    char add[256];
                    sprintf(add, "%u,%u,%u,", x, y, z);
                    fprintf(f_index_array, add);
                    numind += 3;
                }
            }

            // load vertex, normal, color
            if(mode == 1)
            {
                float vx,vy,vz,nx,ny,nz,r,g,b;
                if(sscanf(line, "%f %f %f %f %f %f %f %f %f", &vx, &vy, &vz, &nx, &ny, &nz, &r, &g, &b) == 9)
                {
                    char add[256];

                    sprintf(add, "%.6f,%.6f,%.6f,", vx, vy, vz);
                    fprintf(f_vertex_array, add);
                    numvert++;

                    sprintf(add, "%.6f,%.6f,%.6f,", nx, ny, nz);
                    fprintf(f_normal_array, add);

                    sprintf(add, "%.3f,%.3f,%.3f,", 0.003921569*r, 0.003921569*g, 0.003921569*b);
                    fprintf(f_color_array, add);

                    numcol++;
                    numnorm++;
                }
                else if(sscanf(line, "%f %f %f %f %f %f", &vx, &vy, &vz, &nx, &ny, &nz) == 6)
                {
                    char add[256];

                    sprintf(add, "%.6f,%.6f,%.6f,", vx, vy, vz);
                    fprintf(f_vertex_array, add);
                    numvert++;

                    sprintf(add, "%.6f,%.6f,%.6f,", nx, ny, nz);
                    fprintf(f_normal_array, add);

                    numnorm++;
                }
                else if(sscanf(line, "%f %f %f", &vx, &vy, &vz) == 3)
                {
                    if(vx == 3.0 && vy == 0.0 && vz == 1.0)
                    {
                        fprintf(f_index_array, "0,1,2,");
                        numind += 3;
                        mode = 2;
                        continue;
                    }

                    char add[256];
                    sprintf(add, "%.6f,%.6f,%.6f,", vx, vy, vz);
                    fprintf(f_vertex_array, add);
                    numvert++;
                }
                else
                {
                    fprintf(f_index_array, "0,1,2,");
                    numind += 3;
                    mode = 2;
                    continue;
                }
            }

        }
        fclose(f);
    }

    // finish each output file
    truncLast(f_vertex_array);
    fprintf(f_vertex_array, "};");
    truncLast(f_index_array);
    fprintf(f_index_array, "};");
    truncLast(f_normal_array);
    fprintf(f_normal_array, "};");
    truncLast(f_color_array);
    fprintf(f_color_array, "};");

    // close them
    fclose(f_vertex_array);
    fclose(f_index_array);
    fclose(f_normal_array);
    fclose(f_color_array);
    
    // start output file
    char outfile[256];
    sprintf(outfile, "%s.h", name);
    f = fopen(outfile, "w");
    if(f)
    {
        fprintf(f, "\n#ifndef %s_H\n#define %s_H\n", name, name);
        fclose(f);
    }
    
    // append buffers
    appendFile("vertex_array.txt", outfile);
    appendFile("index_array.txt", outfile);
    if(numnorm > 0)
        appendFile("normal_array.txt", outfile);
    if(numcol > 0)
        appendFile("color_array.txt", outfile);

    // final stat sata
    f = fopen(outfile, "a");
    if(f)
    {
        fprintf(f, "\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n#endif\n", name, numind, name, numvert);
        fclose(f);
    }

    // remove tmp files
    remove("vertex_array.txt");
    remove("index_array.txt");
    remove("normal_array.txt");
    remove("color_array.txt");

    // done
    printf("Output: %s.h\n", name);
    return 0;
}

