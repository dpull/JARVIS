using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Globalization;
using Windows.Graphics.Imaging;
using Windows.Media.Ocr;
using Windows.Storage;
using Windows.Storage.Streams;

namespace Ocr
{
    class WindowsOcr
    {
        private readonly OcrEngine engine;

        public WindowsOcr() : this("zh-Hans-CN")
        {
        }

        public WindowsOcr(string language)
        {
            var lang = new Language(language);
            engine = OcrEngine.TryCreateFromLanguage(lang);
        }

        public async Task<OcrResult> Recognize(SoftwareBitmap bitmap)
        {
            return await engine.RecognizeAsync(bitmap);
        }

        public async Task<OcrResult> Recognize(byte[] image)
        {
            var stream = new InMemoryRandomAccessStream();
            var writer = new DataWriter(stream);
            writer.WriteBytes(image);
            await writer.StoreAsync();
            await stream.FlushAsync();

            var decoder = await BitmapDecoder.CreateAsync(stream);
            var softwareBitmap = await decoder.GetSoftwareBitmapAsync(BitmapPixelFormat.Bgra8, BitmapAlphaMode.Premultiplied);
            return await Recognize(softwareBitmap);
        }

        public async Task<OcrResult> Recognize(string path)
        {
            var storageFile = await StorageFile.GetFileFromPathAsync(path);
            var randomAccessStream = await storageFile.OpenReadAsync();
            var decoder = await BitmapDecoder.CreateAsync(randomAccessStream);
            var softwareBitmap = await decoder.GetSoftwareBitmapAsync(BitmapPixelFormat.Bgra8, BitmapAlphaMode.Premultiplied);
            return await Recognize(softwareBitmap);
        }

        public string ToJSON(OcrResult result)
        {
            var lines = new List<List<word>>();

            foreach (var l in result.Lines)
            {
                var line = new List<word>();
                lines.Add(line);

                foreach (var w in l.Words)
                {
                    line.Add(new word(w));
                }
            }

            return JsonSerializer.Serialize(lines);
        }
    }

    struct word
    {
        public word(in OcrWord w)
        {
            text = w.Text;
            left = w.BoundingRect.Left;
            top = w.BoundingRect.Top;
            right = w.BoundingRect.Right;
            bottom = w.BoundingRect.Bottom;
        }
        public string text { get; }
        public double left { get; }
        public double top { get; }
        public double right { get; }
        public double bottom { get; }
    }
}
