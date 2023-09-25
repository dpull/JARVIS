using System;
using System.Net;
using System.Threading.Tasks;

namespace Ocr
{
    class HttpServer
    {
        private readonly HttpListener listener = new HttpListener();
        private readonly WindowsOcr ocr = new WindowsOcr();

        public HttpServer(string prefix)
        {
            listener.Prefixes.Add(prefix);
        }

        public async Task ListenAndServe()
        {
            listener.Start();

            while (true)
            {
                var context = await listener.GetContextAsync();
                await Process(context);
                context.Response.Close();
            }
        }

        async Task Process(HttpListenerContext context)
        {
            try
            {
                var request = context.Request;
                var response = context.Response;
                var segments = request.Url.Segments;

                if (segments.Length > 1)
                {
                    string action = segments[1].TrimEnd('/');
                    switch (action)
                    {
                        case "get":
                            await HandleGet(request, response);
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception e)
            { 
                Console.WriteLine("Request error: " + e);
            }
        }

        async Task HandleGet(HttpListenerRequest request, HttpListenerResponse response)
        {
            if (!request.HasEntityBody)
                return;

            var text = request.QueryString["text"];
            using (var body = request.InputStream)
            {
                var buffer = new byte[request.ContentLength64];
                int readed = 0;

                while (readed < buffer.Length)
                    readed += await body.ReadAsync(buffer, readed, buffer.Length - readed);

                var result = await ocr.Recognize(buffer);
                var json = ocr.ToJSON(result);
                var jsonBin = System.Text.Encoding.UTF8.GetBytes(json);

                response.ContentType = "text/plain";
                response.ContentLength64 = jsonBin.Length;
                response.StatusCode = 200;
                response.StatusDescription = "OK";

                using (var stream = response.OutputStream)
                {
                    await stream.WriteAsync(jsonBin, 0, jsonBin.Length);
                }
            }
        }
    }
}
