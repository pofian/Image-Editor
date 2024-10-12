// Copyright Filip Popa ~ ACS 313CAb 2023-2024

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
	int rows, cols, rgb, max_color;
	int x1, y1, x2, y2;  // selection coordinates
	int **x;  // image data
	char type[2];  // image type
} image;

typedef struct {
	int word_count;
	char word[100][100];
} command;

// Free memory from image
void free_image(image *img)
{
	if (img->rows == 0)
		return;
	for (int i = 0; i < img->rows; i++)
		free(img->x[i]);
	free(img->x);
	img->rows = 0;
}

// Allocate memory for a new image
void allocate(image *img)
{
	img->x = malloc((img->rows) * sizeof(int *));
	for (int i = 0; i < img->rows; i++)
		img->x[i] = malloc((img->cols) * (img->rgb) * sizeof(int));
}

// x1, y1, x2, y2 are the selection coordinates
void assign_selection(image *img, int a, int b, int c, int d)
{
	img->x1 = a;
	img->y1 = b;
	img->x2 = c;
	img->y2 = d;
}

// Read in normal format if variable binary = 0 and in binary format otherwise
void read(image *img, FILE *file, int binary)
{
	if (binary == 0) {
		for (int i = 0; i < img->rows; i++)
			for (int j = 0; j < img->cols * img->rgb; j++)
				fscanf(file, "%d", &img->x[i][j]);
		return;
	}
	fseek(file, 1, SEEK_CUR);
	unsigned char c;
	for (int i = 0; i < img->rows; i++)
		for (int j = 0; j < img->cols * img->rgb; j++) {
			fread(&c, sizeof(unsigned char), 1, file);
			img->x[i][j] = (int)c;
		}
}

// Load image into memory
void load(image *img, char filename[100])
{
	free_image(img);
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to load %s\n", filename);
		return;
	}
	// Read file in ASCII format or binary format, depending on type
	fscanf(file, "%s", img->type);
	int digit = img->type[1] - '0';  // GRAY RGB
	img->rgb = 1 + ((digit + 1) % 3) * 2;  //  2   3  ASCII
	int binary = (digit - 1) / 3;  //  5   6  BINARY
	fscanf(file, "%d", &img->cols);
	fscanf(file, "%d", &img->rows);
	fscanf(file, "%d", &img->max_color);
	allocate(img);
	read(img, file, binary);
	fclose(file);
	printf("Loaded %s\n", filename);
	assign_selection(img, 0, 0, img->cols, img->rows);
}

// Save image in the file <filename>
void save(image *img, char filename[100], int ascii)
{
	if (!img->rows) {
		printf("No image loaded\n");
		return;
	}
	FILE *file = fopen(filename, "wb");
	if (!file) {
		printf("Failed to open %s\n", filename);
		return;
	}
	if (ascii == 1) {
		if (img->type[1] < '4')
			fprintf(file, "%s\n", img->type);
		else
			fprintf(file, "P%c\n", img->type[1] - 3);
		fprintf(file, "%d %d\n", img->cols, img->rows);
		fprintf(file, "%d\n", img->max_color);
		for (int i = 0; i < img->rows; i++) {
			for (int j = 0; j < img->rgb * img->cols; j++)
				fprintf(file, "%d ", img->x[i][j]);
			fprintf(file, "\n");
		}
	} else {
		if (img->type[1] > '4')
			fprintf(file, "%s\n", img->type);
		else
			fprintf(file, "P%c\n", img->type[1] + 3);
		fprintf(file, "%d %d\n", img->cols, img->rows);
		fprintf(file, "%d\n", img->max_color);
		for (int i = 0; i < img->rows; i++) {
			for (int j = 0; j < img->rgb * img->cols; j++)
				fwrite(&img->x[i][j], sizeof(char), 1, file);
		}
	}
	fclose(file);
	printf("Saved %s\n", filename);
}

// Sort two numbers in ascending order
void sort(int *x, int *y)
{
	if (*x < *y)
		return;
	// Use bitwise operations to ensure no memory overflow
	*x ^= *y;
	*y ^= *x;
	*x ^= *y;
}

int min(int x, int y)
{
	if (x < y)
		return x;
	return y;
}

int max(int x, int y)
{
	if (x > y)
		return x;
	return y;
}

// SELECT command a b c d
void select_subimage(image *img, int a, int b, int c, int d)
{
	sort(&a, &c);
	sort(&b, &d);
	if (a < 0 || c > img->cols || b < 0 || d > img->rows) {
		printf("Invalid set of coordinates\n");
		return;
	}
	if (a == c || b == d) {
		printf("Invalid set of coordinates\n");
		return;
	}
	assign_selection(img, a, b, c, d);
	printf("Selected %d %d %d %d\n", a, b, c, d);
}

// Transform a text into a number
int toint(char word[100], int *x)
{
    *x = 0;
    int i = 0, minus = 1;
    if (word[0] == '-') {
        minus = -1;
        i = 1;
    }
    for (; word[i] != '\0'; i++) {
        // Return 0 if the text is not a number
        if (word[i] > '9' || word[i] < '0')
            return 0;
        *x *= 10;
        *x += (word[i] - '0');
    }
    *x *= minus;
    // Return 1 if the text is indeed a number
    return 1;
}

// CROP command
void crop(image *img)
{
    int a = img->x1, b = img->y1, c = img->x2, d = img->y2;
    int **m = malloc((d - b) * sizeof(int *));
    for (int i = b; i < d; i++) {
        m[i - b] = malloc((c - a) * img->rgb * sizeof(int));
        for (int j = a * img->rgb; j < c * img->rgb; j++)
            m[i - b][j - a * img->rgb] = img->x[i][j];
    }
    free_image(img);
    img->cols = c - a;
    img->rows = d - b;
    img->x = m;
    assign_selection(img, 0, 0, img->cols, img->rows);
    printf("Image cropped\n");
}

// According to the statement, consider only values 0 and 255
int clamp(int x)
{
    if (x > 255)
        return 255;
    if (x < 0)
        return 0;
    return x;
}

// The function calculates the new value of the pixel (i,j) on the color channel
int sum_apply(image *img, int i, int j, int selected, int M[4][3][3], int rgb)
{
    int s = 0;
    for (int k = -1; k <= 1; k++)
        for (int l = -1; l <= 1; l++)
            s += img->x[i + k][3 * (j + l) + rgb] * M[selected][k + 1][l + 1];
    return s;
}

// APPLY command
void apply(image *img, char parameter[100])
{
    int mat[4][3][3] = {{{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}},
                        {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}},
                        {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}},
                        {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}}};
    double sum[4] = {1, 1, 9, 16};
    int selected;
    if (!strcmp(parameter, "EDGE"))
        selected = 0;
    else if (!strcmp(parameter, "SHARPEN"))
        selected = 1;
    else if (!strcmp(parameter, "BLUR"))
        selected = 2;
    else if (!strcmp(parameter, "GAUSSIAN_BLUR"))
        selected = 3;
    else
        selected = 4;
    if (selected == 4) {
        printf("APPLY parameter invalid\n");
        return;
    }
    if (img->rows == 0) {
        printf("No image loaded");
        return;
    }
    if (img->rgb == 1) {
        printf("Easy, Charlie Chaplin\n");
        return;
    }
    int a = img->x1, b = img->y1, c = img->x2, d = img->y2;
    // Calculate in copy[][] the new sub-matrix
    int **copy = malloc((d - b) * sizeof(int *));
    for (int i = 0; i < d - b; i++)
        copy[i] = malloc((c - a) * 3 * sizeof(int));
    // Calculate the new value for each pixel (i,j) and color channel
    // 1<=i<=rows and 1<=j<=cols to avoid being on the edge of the matrix
    for (int i = max(b, 1); i < min(d, img->rows - 1); i++)
        for (int j = max(a, 1); j < min(c, img->cols - 1); j++)
            for (int rgb = 0; rgb <= 2; rgb++) {
                int s = sum_apply(img, i, j, selected, mat, rgb);
                s = round(s / sum[selected]);
                copy[i - img->y1][3 * (j - img->x1) + rgb] = clamp(s);
            }
    // The selected sub-matrix becomes the copy[] matrix
    for (int i = max(b, 1); i < min(d, img->rows - 1); i++)
        for (int j = 3 * max(a, 1); j < 3 * min(c, img->cols - 1); j++)
            img->x[i][j] = copy[i - b][j - 3 * a];
    for (int i = 0; i < d - b; i++)
        free(copy[i]);
    free(copy);
    printf("APPLY %s done\n", parameter);
}

// HISTOGRAM command
void histogram(image *img, int x, int y)
{
    if (img->rgb == 3) {
        printf("Black and white image needed\n");
        return;
    }
    if (y <= 0) {
        printf("Invalid set of parameters\n");
        return;
    }
    // Check if y is a power of 2
    int sy = y;
    while (sy % 2 == 0)
        sy /= 2;
    if (sy != 1) {
        printf("Invalid set of parameters\n");
        return;
    }
    // In v, calculate the frequency of each bin and in maxf the maximum of the values
    int *v = calloc(y, sizeof(int)), maxf = -1;
    for (int i = 0; i < img->rows; i++)
        for (int j = 0; j < img->cols; j++)
            maxf = max(maxf, ++v[img->x[i][j] / (256 / y)]);
    // Print the stars as the problem requires
    for (int i = 0; i < y; i++) {
        v[i] = round((v[i] * x) / maxf);
        printf("%d\t|\t", v[i]);
        for (int j = 0; j < v[i]; j++)
            printf("*");
        printf("\n");
    }
    free(v);
}

void equalize(image *img)
{
    if (img->rgb == 3) {
        printf("Black and white image needed\n");
        return;
    }
    // In v, calculate the frequency of each color
    int *v = calloc(256, sizeof(int)), area = img->rows * img->cols;
    for (int i = 0; i < img->rows; i++)
        for (int j = 0; j < img->cols; j++)
            ++v[img->x[i][j]];
    // v[i] becomes the sum of all frequencies <= i
    for (int i = 1; i < 256; i++)
        v[i] += v[i - 1];
    for (int i = 0; i < img->rows; i++)
        for (int j = 0; j < img->cols; j++)
            img->x[i][j] = round((v[img->x[i][j]] * 255) / area);
    free(v);
    printf("Equalize done\n");
}

// Correction function explained in more detail in README
int cor(int x)
{
    if (x == 1)
        return -2;
    if (x == 2)
        return 2;
    return 0;
}

// Rotation function, explained in detail in README
void rotate(image *img, int angle)
{
	if (angle > 360 || angle < -360 || angle % 90 != 0) {
		printf("Unsupported rotation angle\n");
		return;
	}
	int x1 = img->x1, y1 = img->y1, x2 = img->x2, y2 = img->y2;
	if (x1 + y1 != 0 || x2 + y2 != img->cols + img->rows)
		if (x2 - x1 != y2 - y1) {
			printf("The selection must be square\n");
			return;
		}
	if (angle == 0 || angle == 360 || angle == -360) {
		printf("Rotated %d\n", angle);
		return;
	}
	int colorChannels = img->rgb;
	// Rotate 180 degrees
	if (angle == 180 || angle == -180) {
		int **copy = malloc((y2 - y1) * sizeof(int *));
		for (int i = 0; i < y2 - y1; i++)
			copy[i] = malloc((x2 - x1) * colorChannels * sizeof(int));
		for (int i = 0; i < y2 - y1; i++)
			for (int j = 0; j < colorChannels * (x2 - x1); j++)
				copy[i][j] = img->x[y2 - i - 1][colorChannels * x2 - j - 1];
		for (int i = 0; i < y2 - y1; i++)
			for (int j = 0; j < colorChannels * (x2 - x1); j++)
				img->x[i + y1][colorChannels * x1 + j] = copy[i][j + cor((j + 2) % colorChannels)];
		for (int i = 0; i < y2 - y1; i++)
			free(copy[i]);
		free(copy);
		printf("Rotated %d\n", angle);
		return;
	}
	int **copy = malloc((x2 - x1) * sizeof(int *));
	for (int i = 0; i < x2 - x1; i++)
		copy[i] = malloc((y2 - y1) * colorChannels * sizeof(int));
	// Rotate 90 degrees to the left
	if (angle == -90 || angle == 270)
		for (int i = 0; i < x2 - x1; i++)
			for (int j = 0; j < y2 - y1; j++)
				for (int k = 0; k < colorChannels; k++)
					copy[i][colorChannels * j + k] = img->x[j + y1][colorChannels * (x2 - 1 - i) + k];
	// Rotate 90 degrees to the right
	if (angle == 90 || angle == -270)
		for (int i = 0; i < x2 - x1; i++)
			for (int j = 0; j < y2 - y1; j++)
				for (int k = 0; k < colorChannels; k++)
					copy[i][colorChannels * j + k] = img->x[y2 - 1 - j][colorChannels * (i + x1) + k];
	// Since the selection is complete, I have rotated
	//      the whole image, so img->x becomes copy[][]
	if (x1 + y1 == 0 && x2 + y2 == img->rows + img->cols) {
		free_image(img);
		img->x = copy;
		img->y2 = x2 - x1;
		img->rows = x2 - x1;
		img->x2 = y2 - y1;
		img->cols = y2 - y1;
	} else {
		for (int i = 0; i < y2 - y1; i++)
			for (int j = 0; j < colorChannels * (x2 - x1); j++)
				img->x[i + y1][j + colorChannels * x1] = copy[i][j];
		for (int i = 0; i < x2 - x1; i++)
			free(copy[i]);
		free(copy);
	}
	printf("Rotated %d\n", angle);
}

int verify(int word_count, int x, int y, int rows)
{
	// Check if I have a loaded image
	if (rows == 0) {
		printf("No image loaded\n");
		return 0;
	}
	// Check if the number of words in the read command is appropriate
	if (word_count == x || word_count == y)
		return 1;
	printf("Invalid command\n");
	return 0;
}

// The function processes one command entered
//      from the keyboard, calls the required functions
// Returns 0 if the EXIT command was entered and 1 otherwise
// Explained in more detail in README
int process_command(image *img, command commands)
{
	if (!strcmp(commands.word[0], "LOAD")) {
		if (img->rows == 0 && commands.word_count != 2)
			printf("No image loaded\n");
		else if (verify(commands.word_count, 2, 2, 1))
			load(img, commands.word[1]);
	} else if (!strcmp(commands.word[0], "SELECT")) {
		if (verify(commands.word_count, 2, 5, img->rows)) {
			if (commands.word_count == 2) {
				if (strcmp(commands.word[1], "ALL") == 0) {
					if (img->rows != 0) {
						assign_selection(img, 0, 0, img->cols, img->rows);
						printf("Selected ALL\n");
					} else {
						printf("No image loaded\n");
					}
				} else {
					printf("Invalid command\n");
				}
			}
			if (commands.word_count == 5) {
				int *v = malloc(4 * sizeof(int)), success = 1;
				for (int i = 0; i < 4; i++)
					success *= toint(commands.word[i + 1], &v[i]);
				if (success == 1)
					select_subimage(img, v[0], v[1], v[2], v[3]);
				else
					printf("Invalid command\n");
				free(v);
			}
		}
	} else if (!strcmp(commands.word[0], "HISTOGRAM")) {
		if (verify(commands.word_count, 3, 3, img->rows)) {
			int *v = malloc(2 * sizeof(int)), success = 1;
			for (int i = 0; i < 2; i++)
				success *= toint(commands.word[i + 1], &v[i]);
			if (success == 1)
				histogram(img, v[0], v[1]);
			else
				printf("Invalid command\n");
			free(v);
		}
	} else if (!strcmp(commands.word[0], "EQUALIZE")) {
		if (verify(commands.word_count, 1, 1, img->rows))
			equalize(img);
	} else if (!strcmp(commands.word[0], "ROTATE")) {
		if (verify(commands.word_count, 2, 2, img->rows)) {
			int angle;
			if (!toint(commands.word[1], &angle))
				printf("Invalid command\n");
			else
				rotate(img, angle);
		}
	} else if (!strcmp(commands.word[0], "CROP")) {
		if (verify(commands.word_count, 1, 1, img->rows))
			crop(img);
	} else if (!strcmp(commands.word[0], "APPLY")) {
		if (verify(commands.word_count, 2, 2, img->rows))
			apply(img, commands.word[1]);
	} else if (!strcmp(commands.word[0], "SAVE")) {
		if (verify(commands.word_count, 2, 3, img->rows)) {
			if (commands.word_count == 2)
				save(img, commands.word[1], 0);
			if (commands.word_count == 3) {
				if (strcmp(commands.word[2], "ascii") == 0)
					save(img, commands.word[1], 1);
				else
					printf("Invalid command\n");
			}
		}
	} else if (!strcmp(commands.word[0], "EXIT")) {
		if (img->rows == 0)
			printf("No image loaded\n");
		if (verify(commands.word_count, 1, 1, 1))
			return 0;
	} else {
		printf("Invalid command\n");
	}
	return 1;
}

// The function reads a new line and splits it into words
void read_command(command *commands)
{
	char command[100];
	fgets(command, 100, stdin);
	int count = 0;
	char *p;
	strtok(command, "\n");
	for (p = strtok(command, "\n "); p; p = strtok(NULL, "\n ")) {
		strcpy(commands->word[count], p);
		count++;
	}
	commands->word_count = count;
}

int main(void)
{
	command commands;
	image img;
	img.rows = 0;
	do
		read_command(&commands);
	while (process_command(&img, commands));
	// process_command() returns 0 when EXIT is entered
	free_image(&img);
	return 0;
}
