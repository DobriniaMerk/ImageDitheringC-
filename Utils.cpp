﻿#include <iostream>
#include <algorithm>
#include <iterator>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

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

            //  Temp variables
            int skip = 60;
            int arraySize = (s.x * s.y) / skip;
            int filledRows = 1;
            //  Temp variables

            std::vector< std::vector<sf::Color> > oldColors(colorNum);
            std::vector< std::vector<sf::Color> > newColors(colorNum);
            std::vector< std::vector<sf::Color> > t;

            for (int i = 0; i < colorNum; i++)  // initialize arrays
            {
                oldColors.push_back(std::vector<sf::Color>());
                newColors.push_back(std::vector<sf::Color>());
            }

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
                std::sort(_colors.begin(), _colors.end(), [](sf::Color x, sf::Color y) { return x.r < y.r; });
            else if (g > r && g > b)
                std::sort(_colors.begin(), _colors.end(), [](sf::Color x, sf::Color y) { return x.g < y.g; });
            else if (b > r && b > g)
                std::sort(_colors.begin(), _colors.end(), [](sf::Color x, sf::Color y) { return x.b < y.b; });


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
                    
                    for (int k = 3; k < imgSize; k += 300)
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
                colors = QuantizeMedian(image, colorDepth);

                for (int i = 0; i < colorDepth; i++)
                    std::cout << (int)colors[i].r << ", " << (int)colors[i].g << ", " << (int)colors[i].b << std::endl;

                for (int x = 0; x < image.getSize().x - 1; x++)
                {
                    for (int y = 0; y < image.getSize().y - 1; y++)
                    {
                        sf::Color pix = image.getPixel(x, y);

                        sf::Color wanted = GetNearest(pix, colors, 100000000);
                        

                        image.setPixel(x, y, wanted);

                        sf::Color error = sf::Color(std::clamp(pix.r - wanted.r, 0, 255), std::clamp(pix.g - wanted.g, 0, 255), std::clamp(pix.b - wanted.b, 0, 255));
                       

                        image.setPixel(x + 1, y, Add(Multiply(error, 1 / 7), image.getPixel(x + 1, y)));      //  error distribution
                        image.setPixel(x + 1, y + 1, Add(Multiply(error, 1 / 1), image.getPixel(x + 1, y + 1)));
                        image.setPixel(x, y + 1, Add(Multiply(error, 1 / 5), image.getPixel(x, y + 1)));
                        image.setPixel(x - 1, y + 1, Add(Multiply(error, 1 / 3), image.getPixel(x - 1, y + 1)));
                        
                    }
                }

                return colors;
            }

            /// <summary>
            /// 
            /// </summary>
            /// <param name="img">Image to save</param>
            /// <param name="path">Path to saved image</param>
            /*static void SaveToFile(sf::Image img, sf::Color colors, std::string path = "")
            {
                FileStream fileStream = new FileStream(path, FileMode.Create, FileAccess.Write);
                string write = "";
                int bitsize = (int)Math.Ceiling(Math.Log2((double)colors.Length));

                byte[] bytes = BitConverter.GetBytes(img.Size.X);
                fileStream.WriteByte(bytes[0]);
                fileStream.WriteByte(bytes[1]);
                fileStream.WriteByte(bytes[2]);
                fileStream.WriteByte(bytes[3]);

                bytes = BitConverter.GetBytes(img.Size.Y);
                fileStream.WriteByte(bytes[0]);
                fileStream.WriteByte(bytes[1]);
                fileStream.WriteByte(bytes[2]);
                fileStream.WriteByte(bytes[3]);

                fileStream.WriteByte((byte)colors.Length);  // first byte in file is number of colors
                foreach (Color color in colors) 
                {
                    fileStream.WriteByte(color.R);
                    fileStream.WriteByte(color.G);
                    fileStream.WriteByte(color.B);
                }

                for (int x = 0; x < img.Size.X; x++)
                {
                    for (int y = 0; y < img.Size.Y; y++)
                    {
                        for (byte k = 0; k < colors.Length; k++)
                        {
                            if (colors[k] == img.GetPixel((uint)x, (uint)y))
                            {
                                write += Convert.ToString(k, 2);
                                break;
                            }
                        }
                    }
                }

                Console.WriteLine(write);
            }*/
    };
}
