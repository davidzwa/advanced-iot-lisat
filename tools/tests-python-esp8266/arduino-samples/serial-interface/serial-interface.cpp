// char buffer[32];
// int cnt = 0;
// boolean ready = false;
// int rgbval, red, green, blue;

// void ParseLine(char* buffer)
// {
//     char key[8];
//     char value[8];

//     key = strtok(buffer, "=");  // Everything up to the '=' is the color name
//     value = strtok(NULL, "\n"); // Everything else is the color value
//     if ((key != NULL) && (value != NULL))
//     {
//         rgbval = atoi(value);
//         switch (toupper(key))
//         {
//             case 'R'
//                 red = rgbval;
//                 break;
//             case 'G'
//                 green = rgbval;
//                 break;
//             case 'B'
//                 blue = rgbval;
//                 break;
//         }
//     }
// }
