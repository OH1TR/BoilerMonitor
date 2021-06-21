using OxyPlot;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace BoilerMonitorApp
{
    public partial class MainPage : ContentPage
    {
        private OxyPlot.PlotModel _Model;
        public OxyPlot.PlotModel Model
        {
            get { return _Model; }
            set
            {
                _Model = value;
                OnPropertyChanged("Model");
            }
        }

        public MainPage()
        {
            InitializeComponent();
            
            InitData();
            BindingContext = this;
           
        }

        private Random rand = new Random(0);
        private double[] RandomWalk(int points = 5, double start = 100, double mult = 50)
        {
            // return an array of difting random numbers
            double[] values = new double[points];
            values[0] = start;
            for (int i = 1; i < points; i++)
                values[i] = values[i - 1] + (rand.NextDouble() - .5) * mult;
            return values;
        }

        public void InitData()
        {
            // generate some random Y data
            int pointCount = 5;
            double[] ys1 = RandomWalk(pointCount);
            double[] ys2 = RandomWalk(pointCount);

            // create a series of bars and populate them with data
            var seriesA = new OxyPlot.Series.LineSeries()
            {
                Title = "Series A",
                Color = OxyPlot.OxyColors.Red,
                StrokeThickness = 1
            };

            var seriesB = new OxyPlot.Series.LineSeries()
            {
                Title = "Series B",
                Color = OxyPlot.OxyColors.Blue,
                StrokeThickness = 1
            };

            for (int i = 0; i < pointCount; i++)
            {
                seriesA.Points.Add(new DataPoint(i,ys1[i]));
                seriesB.Points.Add(new DataPoint(i,ys2[i]));
            }

            // create a model and add the bars into it
            var model = new OxyPlot.PlotModel
            {
                Title = "Bar Graph (Column Series)"
            };
            model.Axes.Add(new OxyPlot.Axes.CategoryAxis());
            model.Series.Add(seriesA);
            model.Series.Add(seriesB);

            // load the model into the user control
            Model = model;
        }


    }
}
