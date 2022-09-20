#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#define bp char BREAKPOINT = '1'

namespace ImageDithering
{
    static class Utils
    {
        static sf::Color Divide(sf::Color self, sf::Uint8 n)
        {
            return sf::Color((self.r / n), (self.g / n), (self.b / n));
        }


        static sf::Color Multiply(sf::Color self, sf::Uint8 n)
        {
            return sf::Color((self.r * n), (self.g * n), (self.b * n));
        }

        static sf::Color Add(sf::Color self, sf::Color other)
        {
            return sf::Color(std::clamp(self.r + other.r, 0, 255), std::clamp(self.g + other.g, 0, 255), std::clamp(self.b + other.b, 0, 255));
        }


        /// <summary>
        /// Quatization by median cut
        /// </summary>
        /// <param name="img">Source image</param>
        /// <param name="colorNum">Number of colors to return; Must be a power of two</param>
        /// <returns>Array of Color[colorNum]</returns>
        static std::vector <sf::Color> QuantizeMedian(sf::Image img, int colorNum)
        {
            auto s = img.getSize();

            int skip = 10;
            int filledRows = 1;

            std::vector< std::vector<sf::Color> > oldColors(colorNum);
            std::vector< std::vector<sf::Color> > newColors(colorNum);
            std::vector< std::vector<sf::Color> > t;

            for (int i = 0; i < s.x; i += skip)  // set first array of oldColors to img pixels, with interval of skip
                for(int j = 0; j < s.y; j += skip)
                    oldColors[0].push_back(img.getPixel(i, j));


            while (filledRows < colorNum)  // while not all colors are done
            {
                for (int j = 0; j < filledRows; j++)
                {
                    t = QuantizeMedianSplit(oldColors[j]);  // split each filled row
                    newColors[j * 2] = t[0];
                    newColors[j * 2 + 1] = t[1];  // assign them to newColors
                }

                filledRows *= 2;

                oldColors = newColors;
                for (int y = 0; y < filledRows; y++)
                    newColors[y].clear();
            }

            std::vector<sf::Color> ret(colorNum);  // colors to return
            sf::Vector3f sum = sf::Vector3f(0, 0, 0);
            sf::Color c;
            float n;

            for (int i = 0; i < colorNum; i++)  // calculate mean color of each array and return them
            {
                n = 0;
                sum.x = 0;
                sum.y = 0;
                sum.z = 0;

                for(int j = 0; j < oldColors[i].size(); j++)
                {
                    c = oldColors[i][j];
                    sum.x += c.r;
                    sum.y += c.g;
                    sum.z += c.b;
                    n++;
                }

                sum = sum / n;
                ret[i] = sf::Color(sum.x, sum.y, sum.z);
            }

            for (int i = 0; i < ret.size(); i++)
                std::cout << (int)ret[i].r << ", " << (int)ret[i].g << ", " << (int)ret[i].b << std::endl;

            return ret;
        }


        /// <summary>
        /// Splits "colors" array in halves by maximum color channel
        /// </summary>
        /// <param name="colors">Colors to split</param>
        /// <returns></returns>
        static std::vector<std::vector<sf::Color> > QuantizeMedianSplit(std::vector<sf::Color> _colors)
        {
            std::vector<std::vector<sf::Color> > ret(2);
            std::vector<sf::Color> colors = _colors;
            sf::Color c;
            int r = 0, g = 0, b = 0;

            for (int i = 0; i < colors.size(); i++)
            {
                c = colors[i];
                r += c.r;
                g += c.g;
                b += c.b;
            }

            if (r > g && r > b)
                std::sort(colors.begin(), colors.end(), [](sf::Color x, sf::Color y) { return x.r < y.r; });
            else if (g > r && g > b)
                std::sort(colors.begin(), colors.end(), [](sf::Color x, sf::Color y) { return x.g < y.g; });
            else if (b > r && b > g)
                std::sort(colors.begin(), colors.end(), [](sf::Color x, sf::Color y) { return x.b < y.b; });


            for (int i = 0; i < colors.size(); i++)
            {
                if (i < colors.size() / 2)
                {
                    ret[0].push_back(colors[i]);
                }
                else
                {
                    ret[1].push_back(colors[i]);
                }
            }

            return ret;
        }



        /// <summary>
        /// Splits "colors" array in best point by maximum color channel
        /// </summary>
        /// <param name="colors">Colors[] to split</param>
        /// <returns>sf::Color[][]</returns>
        /*static sf::Color** QuantizeMedianSplitOptimal(sf::Color* _colors, int colorsLength)
        {
            sf::Color* colors = _colors;
            sf::Color** ret = new sf::Color*[2];
            sf::Color c;
            int r = 0, g = 0, b = 0;
            char channel = 'x';

            //foreach (sf::Color c in colors)
            for(int i = 0; i < colorsLength; i++)
            {
                c = colors[i];
                r += c.r;
                g += c.g;
                b += c.b;
            }

            if (r > g && r > b)
            {
                std::vector<sf::Color> myColors;
                for (int i = 0; i < colorsLength; i++)
                {
                    myColors.push_back(colors[i]);
                }
                //OrderBy(order = > order.R)
                std::sort(myColors.begin(),myColors.end(), [](sf::Color x, sf::Color y) { return x.r < y.r; });
                for (int i = 0; i < colorsLength; i++)
                {
                    colors[i] = myColors[i];
                }
                channel = 'r';
            }
            else if (g > r && g > b)
            {
                std::vector<sf::Color> myColors;
                for (int i = 0; i < colorsLength; i++)
                {
                    myColors.push_back(colors[i]);
                }
                std::sort(myColors.begin(), myColors.end(), [](sf::Color x, sf::Color y) { return x.g < y.g; });
                for (int i = 0; i < colorsLength; i++)
                {
                    colors[i] = myColors[i];
                }
                channel = 'g';
            }
            else if (b > r && b > g)
            {
                std::vector<sf::Color> myColors;
                for (int i = 0; i < colorsLength; i++)
                {
                    myColors.push_back(colors[i]);
                }
                std::sort(myColors.begin(), myColors.end(), [](sf::Color x, sf::Color y) { return x.b < y.b; });
                for (int i = 0; i < colorsLength; i++)
                {
                    colors[i] = myColors[i];
                }
                channel = 'b';
            }

            int split = colorsLength / 2;
            int sumFirst = 0, sumSecond = 0;
            int fn = 0, sn = colorsLength;
            int t;
            float maxDiff = 0;


            for (int i = 0; i < colorsLength; i++)
                sumSecond += channel == 'r' ? colors[i].r : channel == 'g' ? colors[i].g : colors[i].b;

            for (int i = 0; i < colorsLength - 1; i+= 1)
            {
                t = channel == 'r' ? colors[i].r : channel == 'g' ? colors[i].g : colors[i].b;
                sn--;
                fn++;
                sumSecond -= t;
                sumFirst += t;

                if (abs(sumFirst/fn - sumSecond/sn) > maxDiff)
                {
                    split = i;
                    maxDiff = abs(sumSecond/fn - sumFirst/sn);
                }
            }

            ret[0] = new sf::Color[split];
            ret[1] = new sf::Color[colorsLength - split];

            for (int i = 0; i < colorsLength; i++)
            {
                if (i < split)
                    ret[0][i] = colors[i];
                else
                    ret[1][i - split] = colors[i];
            }

            return ret;
        }*/


        /// <summary>
        /// Color quantization by clustering (very slow)
        /// </summary>
        /// <param name="img">Sourse image to take colors out</param>
        /// <param name="colorNum">Number of colors to return</param>
        /// <returns>Color[colorNum]</returns>
        static std::vector<sf::Color> Quantize(sf::Image img, int colorNum)
        {
            std::vector<sf::Color> means(colorNum);
            sf::Color color;
            sf::Vector3f sum = sf::Vector3f(0, 0, 0);
            int n, num;

            std::srand(std::time(nullptr));

            means = QuantizeMedian(img, colorNum);

            for (int i = 0; i < 100; i++)
            {
                num = 0;

                //foreach(sf::Color mean in means)
                for(int j = 0; j < colorNum; j++)
                {
                    sum = sum * 0.0f;
                    n = 0;
                    auto s = img.getSize();
                    int imgSize = s.x * s.y;
                    
                    for (int k = 1; k < imgSize; k += 30)
                    {
                        color = sf::Color(img.getPixel(k%s.x, k/s.x));
                        if (GetNearest(color, means, 250) == means[j])
                        {
                            sum.x += color.r;
                            sum.y += color.g;
                            sum.z += color.b;
                            n++;
                        }
                    }

                    if (n != 0)
                    {
                        sum /= (float)n;
                        means[num] = sf::Color(sum.x, sum.y, sum.z);
                    }
                    num++;
                }
            }

            std::cout << "----------------------" << std::endl;
            for (int i = 0; i < means.size(); i++)
                std::cout << (int)means[i].r << ", " << (int)means[i].g << ", " << (int)means[i].b << std::endl;


            return means;
        }


        static float DistanceTo(sf::Color self, sf::Color other)  // to get proper distance you need sqare root of result; not using for optimisation
        {
            return (self.r - other.r) * (self.r - other.r) + (self.g - other.g) * (self.g - other.g) + (self.b - other.b) * (self.b - other.b);
        }


        /// <summary>
        /// Searchs nearest but not farther than maxDist color to color in search array
        /// </summary>
        /// <param name="color">Base color</param>
        /// <param name="search">Array for searching in</param>
        /// <param name="maxDist">Maximum distance of nearest color</param>
        /// <returns>Color</returns>
        static sf::Color GetNearest(sf::Color color, std::vector<sf::Color> search, int maxDist)
        {
            float dist = -1, tDist = 0;
            sf::Color ret = color;
            sf::Color c;

            //foreach (sf::Color c in search)
            for(int i = 0; i < search.size(); i++)
            {
                c = search[i];
                tDist = DistanceTo(color, c);

                if (tDist < maxDist && (dist == -1 || tDist < dist))
                {
                    dist = tDist;
                    ret = c;
                }
            }

            return ret;
        }

        public:
            static std::vector<sf::Color> Dither(sf::Image& image, int colorDepth)
            {
                std::vector<sf::Color> colors;
                colors = Quantize(image, colorDepth);

                for (int x = 0; x < image.getSize().x; x++)
                {
                    for (int y = 0; y < image.getSize().y; y++)
                    {
                        sf::Color pix = image.getPixel(x, y);

                        sf::Color wanted = GetNearest(pix, colors, 100000000);
                        

                        image.setPixel(x, y, wanted);

                        sf::Color error = sf::Color(std::clamp(pix.r - wanted.r, 0, 255), std::clamp(pix.g - wanted.g, 0, 255), std::clamp(pix.b - wanted.b, 0, 255));
                       
                        if (x < image.getSize().x - 1)  //  error distribution;  if's are almost black magic so do not touch without need
                            image.setPixel(x + 1, y, Add(Multiply(error, 1 / 7), image.getPixel(x + 1, y)));
                        if (y < image.getSize().y - 1)
                        {
                            if (x < image.getSize().x - 1)
                                image.setPixel(x + 1, y + 1, Add(Multiply(error, 1 / 1), image.getPixel(x + 1, y + 1)));
                            image.setPixel(x, y + 1, Add(Multiply(error, 1 / 5), image.getPixel(x, y + 1)));
                            image.setPixel(x - 1, y + 1, Add(Multiply(error, 1 / 3), image.getPixel(x - 1, y + 1)));
                        }
                    }
                }

                return colors;
            }

            /// <summary>
            /// 
            /// </summary>
            /// <param name="img">Image to save</param>
            /// <param name="path">Path to saved image</param>
            static void SaveToFile(sf::Image img, std::vector<sf::Color> colors, std::string path = "", std::string filename = "out.fsd")
            {
                std::ofstream filestream(filename, std::ios::in|std::ios::binary|std::ios::trunc);  // std::ios::trunc is for writing file over instead of appending
                sf::Vector2u size = img.getSize();
                filestream.write((char*)&size.x, sizeof(unsigned int));  // first 4 bytes is x of image
                filestream.write((char*)&size.y, sizeof(unsigned int));  // second 4 bytes is y

                char colornum = colors.size();
                filestream.write(&colornum, sizeof(char));               // next byte is number of colors

                std::cout << "-------------------" << std::endl;

                char r, g, b;
                for (int i = 0; i < (int)colornum; i++)                  // then colornum*3 bytes representing colors
                {
                    r = (char)colors[i].r;
                    g = (char)colors[i].g;
                    b = (char)colors[i].b;
                    filestream.write(&r, sizeof(r));
                    filestream.write(&g, sizeof(g));
                    filestream.write(&b, sizeof(b));
                    std::cout << (int)r << ", " << (int)g << ", " << (int)b << std::endl;
                }


                sf::Color pixelColor, color = img.getPixel(0, 0);  // write first pixel in memory
                char rowLength = 1;
                int rowsum = 0;

                int x, y;

                char code, maxrow = static_cast<char>(254);

                for (int n = 1; n < size.x * size.y; n++)
                {
                    int t;
                    
                    x = n % size.x;
                    y = n / size.x;

                    pixelColor = img.getPixel(x, y);
                    if (pixelColor == color && rowLength < maxrow)    // if current pixel color matches color of row     // 255 is reserved
                    {
                        t = rowLength;
                        t++;
                        rowLength = static_cast<char>(t);
                    }
                    else                                           // if not, write current row length and color to file and start new row
                    {
                        code = 0;

                        for (char i = 0; i < colors.size(); i++)   // search for matching color code
                        {
                            if (color == colors[i])
                            {
                                code = i;
                                break;
                            }
                        }

                        filestream.write(&rowLength, sizeof(char));
                        filestream.write(&code, sizeof(char));
                        color = pixelColor;
                        rowsum += rowLength;
                        rowLength = static_cast <char>(1);
                    }
                }

                for (char i = 0; i < colors.size(); i++)   // search for matching color code
                {
                    if (color == colors[i])
                    {
                        code = i;
                        break;
                    }
                }

                filestream.write(&rowLength, sizeof(char));
                filestream.write(&code, sizeof(char));

                std::cout << "Sum of all rowlengths: " << rowsum << "\n";

                filestream.flush();
                filestream.close();
            }


            static sf::Image ReadFile(std::string filename = "out.fsd")
            {
                sf::Image img;
                std::ifstream file(filename, std::ios::out | std::ios::binary);
                unsigned int x, y;
                char colornum;


                file.read((char*)&x, sizeof(unsigned int));
                file.read((char*)&y, sizeof(unsigned int));        // get image x and y size
                file.read(&colornum, sizeof(char));                // number of colors
                img.create(x, y, sf::Color(255, 0, 0));  // filled with red for debugging

                std::cout << "-------------------" << std::endl;
                std::cout << "X: " << x << " Y: " << y << std::endl << "Number of colors: " << (int)colornum << std::endl;


                std::vector<sf::Color> colors;

                for (int i = 0; i < (int)colornum; i++)
                {
                    char r, g, b;
                    file.read(&r, sizeof(char));
                    file.read(&g, sizeof(char));
                    file.read(&b, sizeof(char));
                    colors.push_back(sf::Color(r, g, b));
                }

                int n = 0;
                sf::Color writeColor;

                while (n / x < y)
                {
                    if (file.eof())
                        break;

                    char num = 0, code = 0;
                    file.read(&num, sizeof(char));
                    file.read(&code, sizeof(char));

                    writeColor = colors[code];

                    for (char i = 0; i < num; i++)
                    {
                        img.setPixel(n % x, n / x, writeColor);
                        n++;
                    }

                }
                
                file.close();

                return img;
            }
    };
}
