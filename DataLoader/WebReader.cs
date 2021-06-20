
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;

namespace DataLoader
{
    public delegate void ValuesReceivedEvent(float[] values);

    class WebReader
    {
        private ManualResetEvent _done = new ManualResetEvent(false);
        private int _workerActive = 0;
        private Thread _readThread;

        public event ValuesReceivedEvent OnValuesReceived;

        public void Start()
        {
            this._readThread = new Thread(new ThreadStart(this.Worker));
            this._readThread.Start();
        }

        private void Worker()
        {
            this._workerActive = 1;
            do
            {
                try
                {
                    WebRequest webRequest = WebRequest.Create(ConfigurationManager.AppSettings["IOTAddress"]);
                    webRequest.Method = "GET";
                    WebResponse response = webRequest.GetResponse();
                    using (response)
                    {
                        Stream responseStream = response.GetResponseStream();
                        using (responseStream)
                        {
                            byte[] numArray = new byte[100];
                            int count = responseStream.Read(numArray, 0, numArray.Length);
                            string str = Encoding.ASCII.GetString(numArray, 0, count);
                            if (str.Length > 15)
                            {
                                float[] values = new float[5]
                                {
                  (float) int.Parse(str.Substring(0, 2), NumberStyles.HexNumber),
                  (float) int.Parse(str.Substring(2, 2), NumberStyles.HexNumber),
                  (float) int.Parse(str.Substring(4, 2), NumberStyles.HexNumber),
                  (float) int.Parse(str.Substring(6, 4), NumberStyles.HexNumber),
                  0.0f
                                };
                                if (this.OnValuesReceived != null)
                                    this.OnValuesReceived(values);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine((object)ex);
                }
            }
            while (!this._done.WaitOne(60000, false));
            this._workerActive = 0;
        }

        public void Stop()
        {
            this._done.Set();
            while ((uint)this._workerActive > 0U)
                Thread.Sleep(10);
        }
    }
}
