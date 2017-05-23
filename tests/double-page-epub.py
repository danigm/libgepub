#!/usr/bin/env python3

import sys

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Gepub', '0.4')

from gi.repository import Gtk
from gi.repository import Gepub


class MyWindow(Gtk.Window):

    def __init__(self, book):
        Gtk.Window.__init__(self)

        self.set_default_size(1000, 600)

        self.box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        box2 = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)

        self.w = Gepub.DoubleWidget.new()
        self.w.open(book)

        self.p = Gtk.Button("prev")
        self.n = Gtk.Button("next")
        self.p.connect('clicked', self.prevcb)
        self.n.connect('clicked', self.nextcb)

        self.text = Gtk.Label("%")

        box2.add(self.p)
        box2.add(self.n)
        box2.add(self.text)

        self.box.add(box2)
        self.box.pack_start(self.w, True, True, 0)

        self.add(self.box)

    def prevcb(self, btn):
        self.w.page_prev()

    def nextcb(self, btn):
        self.w.page_next()
        self.text.set_text("%5.2f%% / %s / %s" % (self.w.get_pos(), self.w.get_chapter() +  1, self.w.get_n_chapters()))


if len(sys.argv) != 2:
    print("we need an epub as argument to work")
    sys.exit(0)
book = sys.argv[1]


win = MyWindow(book)
win.connect("delete-event", Gtk.main_quit)
win.show_all()

Gtk.main()
