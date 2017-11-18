using System;
using System.IO;
using System.Net;

namespace TestDataSender
{
    class Program
    {
        static void Main(string[] args)
        {            
            var httpWebRequest = (HttpWebRequest)WebRequest.Create("http://boilermonitorserver.azurewebsites.net/api/values");
            //var httpWebRequest = (HttpWebRequest)WebRequest.Create("http://localhost:52613/api/values");
            httpWebRequest.ContentType = "application/json";
            httpWebRequest.Method = "POST";

            using (var streamWriter = new StreamWriter(httpWebRequest.GetRequestStream()))
            {
                string json = "{\"ID\":\"0\"," +
                              "\"Time\":\"2017-04-23T18:25:43.511Z\"," +
                              "\"Point\":\"1\"," +
                              "\"Value\":\"30\"" +
                              "}";

                streamWriter.Write(json);
                streamWriter.Flush();
                streamWriter.Close();
            }

            var httpResponse = (HttpWebResponse)httpWebRequest.GetResponse();
            using (var streamReader = new StreamReader(httpResponse.GetResponseStream()))
            {
                var result = streamReader.ReadToEnd();
            }
        }
    }
}
