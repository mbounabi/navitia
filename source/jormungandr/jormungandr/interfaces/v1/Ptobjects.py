# coding=utf-8

# Copyright (c) 2001-2014, Canal TP and/or its affiliates. All rights reserved.
#
# This file is part of Navitia,
#     the software to build cool stuff with public transport.
#
# Hope you'll enjoy and contribute to this project,
#     powered by Canal TP (www.canaltp.fr).
# Help us simplify mobility and open public transport:
#     a non ending quest to the responsive locomotion way of traveling!
#
# LICENCE: This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Stay tuned using
# twitter @navitia
# IRC #navitia on freenode
# https://groups.google.com/d/forum/navitia
# www.navitia.io

from flask import Flask, request
from flask.ext.restful import Resource, fields, marshal_with, reqparse, abort
from flask.globals import g
from jormungandr import i_manager, timezone
from jormungandr.interfaces.v1.fields import DisruptionsField
from make_links import add_id_links
from fields import NonNullList, NonNullNested, PbField, error, pt_object, feed_publisher
from ResourceUri import ResourceUri
from make_links import add_id_links
from jormungandr.interfaces.argument import ArgumentDoc
from jormungandr.interfaces.parsers import depth_argument, option_value, default_count_arg_type
from copy import deepcopy


pt_objects = {
    "pt_objects": NonNullList(NonNullNested(pt_object), attribute='places'),
    "disruptions": DisruptionsField,
    "error": PbField(error, attribute='error'),
    "feed_publishers": fields.List(NonNullNested(feed_publisher))
}

pt_object_type_values = ["network", "commercial_mode", "line", "line_group", "route", "stop_area"]


class Ptobjects(ResourceUri):

    def __init__(self, *args, **kwargs):
        ResourceUri.__init__(self, *args, **kwargs)
        self.parsers = {}
        self.parsers["get"] = reqparse.RequestParser(
            argument_class=ArgumentDoc)
        self.parsers["get"].add_argument("q", type=unicode, required=True,
                                         description="The data to search")
        self.parsers["get"].add_argument("type[]", type=option_value(pt_object_type_values),
                                         action="append",default=pt_object_type_values,
                                         description="The type of data to\
                                         search")
        self.parsers["get"].add_argument("count", type=default_count_arg_type, default=10,
                                         description="The maximum number of\
                                         ptobjects returned")
        self.parsers["get"].add_argument("search_type", type=int, default=0,
                                         description="Type of search:\
                                         firstletter or type error")
        self.parsers["get"].add_argument("admin_uri[]", type=unicode,
                                         action="append",
                                         description="If filled, will\
                                         restrained the search within the\
                                         given admin uris")
        self.parsers["get"].add_argument("depth", type=depth_argument,
                                         default=1,
                                         description="The depth of objects")

    @marshal_with(pt_objects)
    def get(self, region=None, lon=None, lat=None):
        self.region = i_manager.get_region(region, lon, lat)
        timezone.set_request_timezone(self.region)
        args = self.parsers["get"].parse_args()
        self._register_interpreted_parameters(args)
        if len(args['q']) == 0:
            abort(400, message="Search word absent")
        response = i_manager.dispatch(args, "pt_objects",
                                      instance_name=self.region)
        return response, 200

