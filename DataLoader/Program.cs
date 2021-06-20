using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Configuration;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;

namespace DataLoader
{
    class Program
    {
        private DateTime lastAddDB = DateTime.MinValue;
        private float[] fireValues = new float[500];
        private CultureInfo invC = CultureInfo.InvariantCulture;
        string OldAPI = ConfigurationManager.AppSettings["OldAPI"];
        string API = ConfigurationManager.AppSettings["API"];
        string IOTAddress = ConfigurationManager.AppSettings["IOTAddress"];


        private static void Main(string[] args)
        {
            ServicePointManager.SecurityProtocol = (SecurityProtocolType)3072;

            ServicePointManager.ServerCertificateValidationCallback = (s, cert, chain, ssl) => true;

            new Program().Start();
        }

        public void Start()
        {
            WebReader webReader = new WebReader();
            webReader.OnValuesReceived += new ValuesReceivedEvent(this.NewLineReceived);
            webReader.Start();
            while (true)
                Thread.Sleep(1000);
        }

        private void NewLineReceived(float[] line)
        {

            float[] numArray = line;
            if (DateTime.Now.Subtract(this.lastAddDB).TotalSeconds > 60.0)
            {
                this.lastAddDB = DateTime.Now;
                try
                {
                    if (!string.IsNullOrEmpty(OldAPI))
                        SendDataOld(numArray[0].ToString((IFormatProvider)this.invC), numArray[2].ToString((IFormatProvider)this.invC), numArray[3].ToString((IFormatProvider)this.invC));

                    if (!string.IsNullOrEmpty(API))
                        SendData(numArray[0], numArray[2], numArray[3]);
                }
                catch (Exception ex)
                {
                    Console.WriteLine((object)ex);
                }
            }

        }


        public void SendDataOld(string value1, string value2, string value3)
        {
            Console.WriteLine("Uploading data.");
            using (WebClient webClient = new WebClient())
                Console.WriteLine("\nResponse received was :\n{0}", (object)Encoding.ASCII.GetString(webClient.UploadValues(OldAPI, new NameValueCollection()
        {
          {
            "machineName",
            Environment.MachineName
          },
          {
            "temp1",
            value1
          },
          {
            "temp2",
            value2
          },
          {
            "temp3",
            value3
          }
        })));
        }

        public void SendData(double value1, double value2, double value3)
        {
            DateTime now = DateTime.UtcNow;

            Temperature[] values = new Temperature[]
            {
                new Temperature() {Point=0,Time=now,Value=value1},
                new Temperature() {Point=2,Time=now,Value=value2},
                new Temperature() {Point=3,Time=now,Value=value3}
            };

            var httpWebRequest = (HttpWebRequest)WebRequest.Create(API);
            httpWebRequest.ContentType = "application/json";
            httpWebRequest.Method = "POST";

            using (var streamWriter = new StreamWriter(httpWebRequest.GetRequestStream()))
            {
                string json = JsonConvert.SerializeObject(values);
                streamWriter.Write(json);
            }

            var httpResponse = (HttpWebResponse)httpWebRequest.GetResponse();
            using (var streamReader = new StreamReader(httpResponse.GetResponseStream()))
            {
                var result = streamReader.ReadToEnd();
            }
        }
    }
}
