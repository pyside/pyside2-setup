Chapter 5 - Add a chart view
=============================

A table is nice to present data, but a chart is even better. For this, you
need the QtCharts module that provides many types of plots and options to
graphically represent data.

The placeholder for a plot is a QChartView, and inside that Widget you can
place a QChart. As a first step, try including only this without any data to
plot.

Make the following highlighted changes to :code:`main_widget.py` from the
previous chapter to add a QChartView:

.. literalinclude:: datavisualize5/main_widget.py
   :linenos:
   :lines: 40-
   :emphasize-lines: 2-3,6,22-36,48-50


