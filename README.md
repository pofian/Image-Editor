**Popa Filip-Andrei**  
**313CAb 2023**

## Image Editor

### Description:

The program reads a command from the keyboard, splitting it into words inside the `read_command()` function, until the `EXIT` command is entered.

Within `read_command()`, I use the `command` structure, which stores both the words of the command and the number of these words.

In the `process_command()` function, I first analyze the first word of the command. Then, if necessary, I call the `verify()` function, which checks:
1. Whether an image is loaded in memory (i.e., `im->rows != 0`).
2. Whether the number of words in the command matches the syntax of the command (e.g., for the `SAVE` command, I need to have 2 or 3 words, so I call `verify(commands.word_count, 2, 3, img->rows)`).

### `load()` Function

The `load()` function reads a new image from a file and saves it in the `img` variable, which is of type `image`. The `image` structure contains:
- The number of rows and columns in the image.
- The maximum value of a color.
- The selection (`in x1, y1, x2, y2`).
- The "magic word" in the `type[]` variable.
- A variable `rgb` that will be 1 if the image is black-and-white, and 3 if the image is colored.
- A dynamically allocated matrix `x[][]`, which stores the numbers read from the file in the same order they appear.

### `save()` Function

The `save()` function saves the corresponding values from the `img` image into a file.

### `select_subimage()` Function

The `select_subimage()` function modifies the values of `im->x1`, `im->y1`, `im->x2`, and `im->y2`.

### `crop()` Function

In the `crop()` function, I create a new matrix `m[][]` whose indices start at 0 and whose values are the same as the selected submatrix. Then, I free the memory allocated for `img->x`, and `img->x` becomes `m`.

### `histogram()` Function

In the `histogram()` function, I construct the frequency vector `v[]` for each bin and store the highest frequency in `maxf`. I then display these frequencies along with asterisks (`*`) based on their values.

### `equalize()` Function

In the `equalize()` function, after constructing the frequency vector `v[]`, I add `v[i-1]` to `v[i]` for any `i`. As a result, `v[i]` will become the sum of all frequencies of pixels that have a value `<= i`.

### `apply()` Function

In the `apply()` function, I copy the selected submatrix into `copy[]`. After this, I calculate the new pixel value for each `i` and `j` inside the selection, for each of the 3 color channels. I only consider the values of `i` and `j` that are **not** on the edges of the matrix (since the new values on the edges are impossible to calculate). Therefore, I use the `min()` and `max()` functions to ensure that `i` is neither 0 nor the number of rows, and `j` is neither 0 nor the number of columns in the matrix.

### `rotate()` Function

The `rotate()` function has three distinct cases (90, 180, and 270 degrees). 

Note: I could have done the 180 and 270-degree rotations as 2 and 3 consecutive 90-degree rotations, respectively, but I chose to treat the 3 cases separately to achieve faster execution times.

For the 180-degree rotation, I rotate the entire matrix, but flipping (r, g, b) upside down results in (b, g, r). Thus, to transform `(0, 1, 2)` into `(0, 1, 2)`, I add `(2, 0, -2)` to the indices. Since I call `cor(j + 2)`, the function takes values `(0, -2, 2)` for `(0, 1, 2)`.

I performed this translation (+2) to ensure that I have `cor(0) = 0`, making the function valid even for non-RGB images, meaning `(j + 2) % rgb = 0` (See line 380 in the program).

For the +/- 90-degree rotation, I calculate the rotated submatrix in the `copy[][]` matrix, which I will then attach to the original matrix. When the selection is not square, it is total, and I do not need to replace all values; it is sufficient for `img->x[]` to become `copy[][]`.
