#include <iostream>
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
        /*static sf::Color* QuantizeMedian(sf::Image img, int colorNum)
        {
            sf::Color** oldColors = new sf::Color*[colorNum];
            sf::Color** newColors = new sf::Color*[colorNum];
            sf::Color** t = new sf::Color*[colorNum];

            auto s = img.getSize();

            //  Temp variables
            int skip = 70;
            int arraySize = (s.x * s.y) / skip;
            int filledRows = 1;
            //  Temp variables

            for (int i = 0; i < colorNum; i++)  // initialize arrays
            {
                newColors[i] = new sf::Color[arraySize];
                oldColors[i] = new sf::Color[arraySize];
            }

            for (int i = 0; i < s.x; i += skip)  // set first array of oldColors to img pixels, with interval of skip
                for(int j = 0; j < s.y; j += skip)
                    oldColors[0][i] = img.getPixel(i, j);

            while (filledRows < colorNum)  // while not all colors are done
            {
                for (int j = 0; j < filledRows; j++)
                {
                    t = QuantizeMedianSplitOptimal(oldColors[j], arraySize);  // split each filled row
                    newColors[j * 2] = t[0];
                    newColors[j * 2 + 1] = t[1];  // assign them to newColors
                }

                filledRows *= 2;

                for (int y = 0; y < filledRows; y++)
                {;
                    std::copy(newColors[y], newColors[y] + arraySize * sizeof(sf::Color), oldColors[y]);  //  ATTENTION, MAY NOT WORK
                    //std::copy(newColors[y], newColors[y] + arraySize, oldColors[y]); // if upper not working
                    //oldColors[y] = newColors[y].Clone();  // copy newColors to oldColors
                    newColors[y] = new sf::Color[arraySize];
                }

                std::cout << filledRows << std::endl;
            }

            sf::Color* ret = new sf::Color[colorNum];  // colors to return
            sf::Vector3f sum = sf::Vector3f(0, 0, 0);
            sf::Color c;

            for (int i = 0; i < colorNum; i++)  // calculate mean color of each array and return them
            {
                float n = 0;
                //foreach (sf::Color c in oldColors[i])
                for(int j = 0; j < arraySize; j++)
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
        }*/


        /// <summary>
        /// Splits "colors" array in best point by maximum color channel
        /// </summary>
        /// <param name="colors">Colors[] to split</param>
        /// <returns>sf::Color[][]</returns>
        /*static sf::Color** QuantizeMedianSplitOptimal(sf::Color* colors, int colorsLength)
        {
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
                colors = colors.OrderBy(order => order.R).ToArray();
                channel = 'r';
            }
            else if (g > r && g > b)
            {
                colors = colors.OrderBy(order => order.G).ToArray();
                channel = 'g';
            }
            else if (b > r && b > g)
            {
                colors = colors.OrderBy(order => order.B).ToArray();
                channel = 'b';
            }

            int split = colors.Length / 2;
            int sumFirst = 0, sumSecond = 0;
            int fn = 0, sn = colors.Length;
            int t;
            float maxDiff = 0;


            for (int i = 0; i < colors.Length; i++)
                sumSecond += channel == 'r' ? colors[i].R : channel == 'g' ? colors[i].G : colors[i].B;

            for (int i = 0; i < colors.Length - 1; i+= 1)
            {
                t = channel == 'r' ? colors[i].R : channel == 'g' ? colors[i].G : colors[i].B;
                sn--;
                fn++;
                sumSecond -= t;
                sumFirst += t;

                if (MathF.Abs(sumFirst/fn - sumSecond/sn) > maxDiff)
                {
                    split = i;
                    maxDiff = MathF.Abs(sumSecond/fn - sumFirst/sn);
                }
            }

            ret[0] = colors.Take(colors.Length / 2).ToArray();
            ret[1] = colors.Skip(colors.Length / 2).ToArray();

            return ret;
        }*/


        /// <summary>
        /// Color quantization by clustering (very slow)
        /// </summary>
        /// <param name="img">Sourse image to take colors out</param>
        /// <param name="colorNum">Number of colors to return</param>
        /// <returns>Color[colorNum]</returns>
        static sf::Color* Quantize(sf::Image img, int colorNum)
        {
            sf::Color* means = new sf::Color[colorNum];
            sf::Color color;
            sf::Vector3f sum = sf::Vector3f(0, 0, 0);
            int n, num;

            for (int i = 0; i < colorNum; i++)
                means[i] = sf::Color(std::rand()%255, std::rand() % 255, std::rand() % 255);

            for (int i = 0; i < 10; i++)
            {
                num = 0;

                //foreach(sf::Color mean in means)
                for(int j = 0; j < colorNum; j++)
                {
                    std::cout << num << std::endl;
                    sum = sum * 0.0f;
                    n = 0;
                    auto s = img.getSize();
                    int imgSize = s.x * s.y;
                    
                    for (int k = 3; k < imgSize; k += 300)
                    {
                        color = sf::Color(img.getPixel(k%s.x, k/s.x));
                        if (GetNearest(color, means, 250, colorNum) == means[j])
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
        static sf::Color GetNearest(sf::Color color, sf::Color* search, int maxDist, int searchSize)
        {
            float dist = -1, tDist = 0;
            sf::Color ret = color;
            sf::Color c;

            //foreach (sf::Color c in search)
            for(int i = 0; i < searchSize; i++)
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
            static sf::Color* Dither(sf::Image _image, int colorDepth)
            {
                sf::Image image = _image;
                sf::Color* colors;
                colors = Quantize(image, colorDepth);

                for (int x = 0; x < image.getSize().x; x++)
                {
                    for (int y = 0; y < image.getSize().y; y++)
                    {
                        sf::Color pix = image.getPixel(x, y);

                        sf::Color wanted = GetNearest(pix, colors, 100000000, colorDepth);
                    

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
