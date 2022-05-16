#include <iostream>
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
        static sf::Color[] QuantizeMedian(Image img, int colorNum)  // unfinished
        {
            Color[][] oldColors = new Color[colorNum][];
            Color[][] newColors = new Color[colorNum][];
            Color[][] t = new Color[colorNum][];
            oldColors[0] = new Color[img.Pixels.Length / 3];

            //  Temp variables
            int skip = 300;
            int arraySize = (img.Pixels.Length / 3) / skip;
            int filledRows = 1;
            //  Temp variables

            for (int i = 0; i < colorNum; i++)  // initialize arrays
            {
                newColors[i] = new Color[arraySize];
                oldColors[i] = new Color[arraySize];
            }

            for (int i = 0; i < arraySize; i++)  // set first array of oldColors to img pixels, with interval of skip
                oldColors[0][i] = new Color(img.Pixels[skip * i], img.Pixels[skip * i + 1], img.Pixels[skip * i + 2]);

            while (filledRows < colorNum)  // while not all colors are done
            {
                for (int j = 0; j < filledRows; j++)
                {
                    t = QuantizeMedianSplitOptimal(oldColors[j]);  // split each filled row
                    newColors[j * 2] = t[0];
                    newColors[j * 2 + 1] = t[1];  // assign them to newColors
                }

                filledRows *= 2;

                for (int y = 0; y < filledRows; y++)
                {
                    oldColors[y] = (Color[])newColors[y].Clone();  // copy newColors to oldColors
                    newColors[y] = new Color[arraySize];
                }

                Console.WriteLine(filledRows);
            }

            Color[] ret = new Color[colorNum];  // colors to return
            Vector3f sum = new Vector3f(0, 0, 0);

            for (int i = 0; i < colorNum; i++)  // calculate mean color of each array and return them
            {
                int n = 0;
                foreach (Color c in oldColors[i])
                {
                    sum.X += c.R;
                    sum.Y += c.G;
                    sum.Z += c.B;
                    n++;
                }

                sum /= n;
                ret[i] = new Color((byte)sum.X, (byte)sum.Y, (byte)sum.Z);
            }

            return ret;
        }

        /// <summary>
        /// Splits "colors" array in halves by maximum color channel
        /// </summary>
        /// <param name="colors">Colors to split</param>
        /// <returns></returns>
        static Color[][] QuantizeMedianSplit(Color[] colors)
        {
            Color[][] ret = new Color[2][];
            ret[0] = new Color[colors.Length/2];
            ret[1] = new Color[colors.Length / 2];
            int r = 0, g = 0, b = 0;

            foreach (Color c in colors)
            {
                r += c.R;
                g += c.G;
                b += c.B;
            }

            if (r > g && r > b)
            {
                colors = colors.OrderBy(order => order.R).ToArray();
            }
            else if (g > r && g > b)
            {
                colors = colors.OrderBy(order => order.G).ToArray();
            }
            else if (b > r && b > g)
            {
                colors = colors.OrderBy(order => order.B).ToArray();
            }

            ret[0] = colors.Take(colors.Length / 2).ToArray();
            ret[1] = colors.Skip(colors.Length / 2).ToArray();

            return ret;
        }


        /// <summary>
        /// Splits "colors" array in best point by maximum color channel
        /// </summary>
        /// <param name="colors">Colors to split</param>
        /// <returns></returns>
        static Color[][] QuantizeMedianSplitOptimal(Color[] colors)
        {
            Color[][] ret = new Color[2][];
            int r = 0, g = 0, b = 0;
            char channel = 'x';

            foreach (Color c in colors)
            {
                r += c.R;
                g += c.G;
                b += c.B;
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
        }


        /// <summary>
        /// Color quantization by clustering (very slow)
        /// </summary>
        /// <param name="img">Sourse image to take colors out</param>
        /// <param name="colorNum">Number of colors to return</param>
        /// <returns>Color[colorNum]</returns>
        public static Color[] Quantize(Image img, int colorNum)
        {
            Random random = new Random();
            Color[] means = new Color[colorNum];
            Color color;
            Vector3f sum = new Vector3f(0, 0, 0);
            int n, j;

            for (int i = 0; i < colorNum; i++)
                means[i] = new Color((byte)random.Next(255), (byte)random.Next(255), (byte)random.Next(255));

            for (int i = 0; i < 10; i++)
            {
                j = 0;

                foreach(Color mean in means)
                {
                    Console.WriteLine(j);
                    sum *= 0;
                    n = 0;

                    for (int k = 3; k < img.Pixels.Length; k += 300)
                    {
                        color = new Color(img.Pixels[k], img.Pixels[k - 1], img.Pixels[k - 2]);
                        if (GetNearest(color, means, 250) == mean)
                        {
                            sum.X += color.R;
                            sum.Y += color.G;
                            sum.Z += color.B;
                            n++;
                        }
                    }

                    if (n != 0)
                    {
                        sum /= n;
                        means[j] = new Color((byte)sum.X, (byte)sum.Y, (byte)sum.Z);
                    }
                    j++;
                }
            }

            return means;
        }


        static float DistanceTo(this Color self, Color other)  // to get proper distance you need sqare root of result; not using for optimisation
        {
            return (self.R - other.R) * (self.R - other.R) + (self.G - other.G) * (self.G - other.G) + (self.B - other.B) * (self.B - other.B);
        }


        /// <summary>
        /// Searchs nearest but not farther than maxDist color to color in search array
        /// </summary>
        /// <param name="color">Base color</param>
        /// <param name="search">Array for searching in</param>
        /// <param name="maxDist">Maximum distance of nearest color</param>
        /// <returns></returns>
        static sf::Color GetNearest(sf::Color color, sf::Color[] search, int maxDist)
        {
            float dist = -1, tDist = 0;
            sf::Color ret = color;

            foreach (sf::Color c in search)
            {
                tDist = color.DistanceTo(c);

                if (tDist < maxDist && (dist == -1 || tDist < dist))
                {
                    dist = tDist;
                    ret = c;
                }
            }

            return ret;
        }

        static sf::Color* Dither(sf::Image _image, int colorDepth, bool clustering = false)
        {
            sf::Image image = _image;
            //sf::Color[] colors;
            if (clustering)
                sf::Color[] colors = Quantize(image, colorDepth);
            else
                sf::Color[] colors = QuantizeMedian(image, colorDepth);

            for (sf::Uint8 x = 0; x < image.getSize().x; x++)
            {
                for (sf::Uint8 y = 0; y < image.getSize().y; y++)
                {
                    sf::Color pix = image.getPixel(x, y);

                    sf::Color wanted = GetNearest(pix, colors, 100000000);
                    

                    image.setPixel(x, y, wanted);

                    sf::Color error = sf::Color(std::clamp(pix.r - wanted.r, 0, 255), std::clamp(pix.g - wanted.g, 0, 255), std::clamp(pix.b - wanted.b, 0, 255));

                    image.setPixel(x + 1, y, Multiply(error, 1 / 7).Add(image.getPixel(x + 1, y)));      //  error distribution
                    image.setPixel(x + 1, y + 1, Multiply(error, 1 / 1).Add(image.getPixel(x + 1, y + 1)));
                    image.setPixel(x, y + 1, Multiply(error, 1 / 5).Add(image.getPixel(x, y + 1)));
                    image.setPixel(x - 1, y + 1, Multiply(error, 1 / 3).Add(image.getPixel(x - 1, y + 1)));
                }
            }

            return colors;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="img">Image to save</param>
        /// <param name="path">Path to saved image</param>
        static public void SaveToFile(Image img, Color[] colors, string path = "")
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
        }
    }
}
