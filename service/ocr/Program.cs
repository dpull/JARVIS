using System;
using System.IO;
using System.Threading.Tasks;

namespace Ocr
{
    class Program
    {
        static async Task Main(string[] args)
        {
            try
            {
                switch (args.Length)
                {
                    case 1:
                        await get(args[0]);
                        break;

                    default:
                        await serve();
                        break;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("ERROR: " + e);
            }
        }

        static async Task serve()
        {
            var server = new HttpServer("http://localhost:7788/");
            await server.ListenAndServe();
        }

        static async Task get(string path)
        {
            string[] files;
            if (Directory.Exists(path))
                files = Directory.GetFiles(path);
            else if (File.Exists(path))
                files = new string[] { path };
            else
                throw new ArgumentException("path");

            var ocr = new WindowsOcr();

            foreach (var file in files)
            {
                var result = await ocr.Recognize(file);
                var json = ocr.ToJSON(result);
                Console.WriteLine("[{0}]\n\t{1}", file, json);
            }
        }
    }
}
