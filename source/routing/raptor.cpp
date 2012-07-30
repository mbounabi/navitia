#include "raptor.h"
namespace navitia { namespace routing { namespace raptor {

communRAPTOR::communRAPTOR(navitia::type::Data &data) : data(data)
{
    //Construction de la liste des marche à pied
    BOOST_FOREACH(navitia::type::Connection connection, data.pt_data.connections) {
        foot_path[connection.departure_stop_point_idx].push_back(connection.idx);
    }
}

std::pair<unsigned int, bool> communRAPTOR::earliest_trip(unsigned int route, unsigned int stop_area, map_int_pint_t &best, unsigned int count){
    if(best.count(stop_area) == 0)
        return std::pair<unsigned int, bool>(-1, false);

    if(count > 1) {
        if((best[stop_area].dt + 2*60).date > best[stop_area].dt.date)
            return std::make_pair(earliest_trip(route, stop_area, best[stop_area].dt + 2*60).first, true);
        else {
            return earliest_trip(route, stop_area, best[stop_area].dt + 2*60);
        }
    } else
        return earliest_trip(route, stop_area, best[stop_area].dt);
}


std::pair<unsigned int, bool> communRAPTOR::earliest_trip(unsigned int route, unsigned int stop_area, map_retour_t &retour, unsigned int count){
    if(retour[count - 1].count(stop_area) == 0)
        return std::pair<unsigned int, bool>(-1, false);

    if(count > 1) {
        if((retour[count-1][stop_area].dt + 2*60).date > retour[count-1][stop_area].dt.date)
            return std::make_pair(earliest_trip(route, stop_area, retour[count-1][stop_area].dt + 2*60).first, true);
        else
            return earliest_trip(route, stop_area, retour[count-1][stop_area].dt + 2*60);
    } else
        return earliest_trip(route, stop_area, retour[count-1][stop_area].dt);
}


std::pair<unsigned int, bool> communRAPTOR::earliest_trip(unsigned int route, unsigned int stop_area, DateTime dt){

    bool pam = false;
    if(dt.hour > 86400) {
        dt.hour = dt.hour %86400;
        ++dt.date;
        pam = true;
    }

    int rp_id = get_rp_id(route, stop_area);
    if(rp_id == -1)
        return std::pair<unsigned int, bool>(-1, false);

    for(auto it = std::lower_bound(data.pt_data.route_points[rp_id].vehicle_journey_list.begin(),
                                   data.pt_data.route_points[rp_id].vehicle_journey_list.end(),
                                   dt.hour, compare_rp(data.pt_data.route_points.at(rp_id), data));
        it != data.pt_data.route_points[rp_id].vehicle_journey_list.end(); ++it) {
        navitia::type::ValidityPattern vp = data.pt_data.validity_patterns[data.pt_data.vehicle_journeys[*it].validity_pattern_idx];

        if(vp.check(dt.date)) {
            return std::pair<unsigned int, bool>(*it, pam);
        }
    }
    ++dt.date;

    for(auto  it = std::lower_bound(data.pt_data.route_points[rp_id].vehicle_journey_list.begin(),
                                    data.pt_data.route_points[rp_id].vehicle_journey_list.end(),
                                    0, compare_rp(data.pt_data.route_points[rp_id], data));
        it != data.pt_data.route_points[rp_id].vehicle_journey_list.end(); ++it) {
        navitia::type::ValidityPattern vp = data.pt_data.validity_patterns[data.pt_data.vehicle_journeys[*it].validity_pattern_idx];
        if(vp.check(dt.date)) {
            return std::pair<unsigned int, bool>(*it, true);
        }
    }
    return std::pair<unsigned int, bool>(-1, false);
}


std::pair<unsigned int, bool> communRAPTOR::tardiest_trip(unsigned int route, unsigned int stop_area, map_int_pint_t &best, unsigned int count){
    if(best.count(stop_area) == 0)
        return std::pair<unsigned int, bool>(-1, false);

    if(count > 1) {
        if((best[stop_area].dt + 2*60).date > best[stop_area].dt.date)
            return std::make_pair(tardiest_trip(route, stop_area, best[stop_area].dt + 2*60).first, true);
        else {
            return tardiest_trip(route, stop_area, best[stop_area].dt + 2*60);
        }
    } else
        return tardiest_trip(route, stop_area, best[stop_area].dt);
}


std::pair<unsigned int, bool> communRAPTOR::tardiest_trip(unsigned int route, unsigned int stop_area, map_retour_t &retour, unsigned int count){
    if(retour[count - 1].count(stop_area) == 0)
        return std::pair<unsigned int, bool>(-1, false);

    if(count > 1) {
        if((retour[count-1][stop_area].dt + 2*60).date > retour[count-1][stop_area].dt.date)
            return std::make_pair(tardiest_trip(route, stop_area, retour[count-1][stop_area].dt + 2*60).first, true);
        else
            return tardiest_trip(route, stop_area, retour[count-1][stop_area].dt + 2*60);
    } else
        return tardiest_trip(route, stop_area, retour[count-1][stop_area].dt);
}


std::pair<unsigned int, bool> communRAPTOR::tardiest_trip(unsigned int route, unsigned int stop_area, DateTime dt){

    bool pam = false;
    if(dt.hour > 86400) {
        dt.normalize();
        pam = true;
    }

    int rp_id = get_rp_id(route, stop_area);
    if(rp_id == -1)
        return std::pair<unsigned int, bool>(-1, false);


    for(auto it = std::upper_bound(data.pt_data.route_points[rp_id].vehicle_journey_list.begin(),
                                   data.pt_data.route_points[rp_id].vehicle_journey_list.end(),
                                   dt.hour, compare_rp_reverse(data.pt_data.route_points.at(rp_id), data));
        it != data.pt_data.route_points[rp_id].vehicle_journey_list.end(); ++it) {
        navitia::type::ValidityPattern vp = data.pt_data.validity_patterns[data.pt_data.vehicle_journeys[*it].validity_pattern_idx];

        if(vp.check(dt.date)) {
            return std::pair<unsigned int, bool>(*it, pam);
        }
    }
    --dt.date;

    for(auto  it = std::upper_bound(data.pt_data.route_points[rp_id].vehicle_journey_list.begin(),
                                    data.pt_data.route_points[rp_id].vehicle_journey_list.end(),
                                    0, compare_rp_reverse(data.pt_data.route_points[rp_id], data));
        it != data.pt_data.route_points[rp_id].vehicle_journey_list.end(); ++it) {
        navitia::type::ValidityPattern vp = data.pt_data.validity_patterns[data.pt_data.vehicle_journeys[*it].validity_pattern_idx];
        if(vp.check(dt.date)) {
            return std::pair<unsigned int, bool>(*it, true);
        }
    }
    return std::pair<unsigned int, bool>(-1, false);
}




int communRAPTOR::get_rp_id(unsigned int route, unsigned int stop_area) {
    return get_rp_id(data.pt_data.routes[route], stop_area);
}

int communRAPTOR::get_rp_id(const navitia::type::Route &route, unsigned int stop_area) {
    for(unsigned int i = 0; i < route.route_point_list.size();++i)
        if(data.pt_data.stop_points[data.pt_data.route_points[route.route_point_list[i]].stop_point_idx].stop_area_idx == stop_area)
            return route.route_point_list[i];
    return -1;
}

int communRAPTOR::get_rp_order(unsigned int route, unsigned int stop_area) {
    return get_rp_order(data.pt_data.routes[route], stop_area);
}

int communRAPTOR::get_rp_order(const navitia::type::Route &route, unsigned int stop_area) {
    for(unsigned int i = 0; i < route.route_point_list.size();++i)
        if(data.pt_data.stop_points[data.pt_data.route_points[route.route_point_list[i]].stop_point_idx].stop_area_idx== stop_area)
            return i;
    return -1;
}

int communRAPTOR::get_sa_rp(unsigned int order, int route) {
    return data.pt_data.stop_points[data.pt_data.route_points[data.pt_data.routes[route].route_point_list[order]].stop_point_idx].stop_area_idx;
}






map_int_int_t communRAPTOR::make_queue(std::vector<unsigned int> stops) {
    map_int_int_t retour;
    map_int_int_t::iterator it;
    BOOST_FOREACH(unsigned int said, stops) {
        BOOST_FOREACH(unsigned int spid, data.pt_data.stop_areas[said].stop_point_list) {
            BOOST_FOREACH(unsigned int rp, data.pt_data.stop_points[spid].route_point_list) {
                if(retour.count(data.pt_data.route_points[rp].route_idx) == 0 ||
                        data.pt_data.route_points[rp].order < get_rp_order(data.pt_data.route_points[rp].route_idx, retour[data.pt_data.route_points[rp].route_idx]))
                    retour[data.pt_data.route_points[rp].route_idx] = said;
            }
        }
    }

    return retour;
}




Path monoRAPTOR::compute_raptor(map_int_pint_t departs, map_int_pint_t destinations) {
    map_retour_t retour;
    map_int_pint_t best;
    std::vector<unsigned int> marked_stop;


    BOOST_FOREACH(auto item, departs) {
        retour[0][item.first] = item.second;
        best[item.first] = item.second;
        marked_stop.push_back(item.first);
    }

    best_dest b_dest;

    BOOST_FOREACH(auto item, destinations) {
        b_dest.ajouter_destination(item.first, item.second);
    }

    unsigned int count = 1;


    boucleRAPTOR(marked_stop, retour, best, b_dest, count);



    if(b_dest.best_now != type_retour()) {
        unsigned int destination_idx = b_dest.best_now_said;
        return makeBestPath(retour, best, departs, destination_idx, count);
    }
    Path result;
    return result;
}


void RAPTOR::boucleRAPTOR(std::vector<unsigned int> &marked_stop, map_retour_t &retour, map_int_pint_t &best, best_dest &b_dest, unsigned int & count) {
    map_int_int_t Q;

    int et_temp ;

    int t, stid, said;
    unsigned int route, p;
    bool pam;
    while(((Q.size() > 0) /*& (count < 10)*/) || count == 1) {

        Q = make_queue(marked_stop);
        marked_stop.clear();

        BOOST_FOREACH(map_int_int_t::value_type vq, Q) {
            route = vq.first;
            p = vq.second;
            t = -1;
            int temps_depart = retour[count - 1][p].dt.hour;
            int working_date = retour[count - 1][p].dt.date;
            int prev_temps = temps_depart;
            stid = -1;
            int embarquement = p;
            for(unsigned int i = get_rp_order(route, p); i < data.pt_data.routes[route].route_point_list.size(); ++i) {
                said = get_sa_rp(i, route);

                if(t != -1) {
                    stid = data.pt_data.vehicle_journeys[t].stop_time_list[i];
                    temps_depart = data.pt_data.stop_times[stid].departure_time%86400;
                    //Passe minuit
                    if(prev_temps > (data.pt_data.stop_times[stid].arrival_time%86400))
                        ++working_date;
                    //On stocke, et on marque pour explorer par la suite
                    type_retour min_said;
                    if(best.count(said) > 0)
                        min_said = std::min(best[said], b_dest.best_now);
                    else
                        min_said = b_dest.best_now;
                    if(type_retour(-1, DateTime(working_date,data.pt_data.stop_times[stid].arrival_time%86400)) < min_said) {
                        retour[count][said]  = type_retour(stid, embarquement, DateTime(working_date, data.pt_data.stop_times[stid].arrival_time%86400));
                        best[said] = type_retour(stid, DateTime(working_date, data.pt_data.stop_times[stid].arrival_time%86400));
                        b_dest.ajouter_best(said, type_retour(stid, DateTime(working_date, data.pt_data.stop_times[stid].arrival_time%86400)));
                        if(std::find(marked_stop.begin(), marked_stop.end(), said) == marked_stop.end()) {
                            marked_stop.push_back(said);
                        }
                    }
                }

                //Si on peut arriver plus tôt à l'arrêt en passant par une autre route
                if(retour[count-1].count(said) > 0)  {
                    if(retour[count-1][said].dt <= DateTime(working_date, temps_depart)){
                        std::tie(et_temp, pam) = earliest_trip(route, said, retour, count);
                        if((et_temp >=0) && (et_temp != t)) {

                            t = et_temp;
                            working_date = retour[count -1][said].dt.date;
                            stid = data.pt_data.vehicle_journeys[t].stop_time_list[i];
                            embarquement = said;
                            if(pam || data.pt_data.stop_times.at(stid).arrival_time > 86400)
                                ++working_date;
                        }
                    }
                }
                if(stid != -1)
                    prev_temps = data.pt_data.stop_times[stid].arrival_time % 86400;
            }
        }


        std::vector<unsigned int> marked_stop_copy;
        marked_stop_copy = marked_stop;
        BOOST_FOREACH(auto stop_area, marked_stop_copy) {
            BOOST_FOREACH(auto stop_p, data.pt_data.stop_areas.at(stop_area).stop_point_list) {
                auto it_fp = foot_path.find(stop_p);
                if(it_fp != foot_path.end()) {
                    BOOST_FOREACH(auto connection_idx, (*it_fp).second) {

                        unsigned int saiddest = data.pt_data.stop_points.at(data.pt_data.connections[connection_idx].destination_stop_point_idx).stop_area_idx;

                        if(best.count(saiddest) > 0)
                            best[saiddest] = std::min(best[saiddest], type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt + data.pt_data.connections[connection_idx].duration), connection));
                        else
                            best[saiddest] = type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt + data.pt_data.connections[connection_idx].duration), connection);
                        if(retour[count].count(saiddest) > 0)
                            retour[count][saiddest] = std::min(retour[count][saiddest], type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt + data.pt_data.connections[connection_idx].duration), connection));
                        else
                            retour[count][saiddest] = type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt + data.pt_data.connections[connection_idx].duration), connection);
                        b_dest.ajouter_best(saiddest, type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt + data.pt_data.connections[connection_idx].duration), connection));
                        marked_stop.push_back(saiddest);
                    }
                }
            }
        }



        ++count;
    }

}


void reverseRAPTOR::boucleRAPTOR(std::vector<unsigned int> &marked_stop, map_retour_t &retour, map_int_pint_t &best, best_dest &b_dest, unsigned int & count) {
    map_int_int_t Q;

    int et_temp ;

    int t, stid, said;
    unsigned int route, p;
    bool pam;
    b_dest.reverse();

    //    std::cout << "retour 0 1 " << retour.at(0).at(1).dt << std::endl;
    while(((Q.size() > 0) /*& (count < 10)*/) || count == 1) {

        Q = make_queue(marked_stop);
        marked_stop.clear();

        BOOST_FOREACH(map_int_int_t::value_type vq, Q) {
            route = vq.first;
            p = vq.second;
            t = -1;
            int temps_depart = retour[count - 1][p].dt.hour;
            int working_date = retour[count - 1][p].dt.date;
            int prev_temps = temps_depart;
            stid = -1;
            int embarquement = p;
            for(int i = get_rp_order(route, p); i >= 0; --i) {
                said = get_sa_rp(i, route);

                if(t != -1) {
                    stid = data.pt_data.vehicle_journeys[t].stop_time_list[i];
                    temps_depart = data.pt_data.stop_times[stid].departure_time%86400;
                    //Passe minuit
                    if(prev_temps < (data.pt_data.stop_times[stid].arrival_time%86400))
                        --working_date;
                    //On stocke, et on marque pour explorer par la suite
                    type_retour max_said;
                    if(best.count(said) > 0)
                        max_said = std::max(best[said], b_dest.best_now);
                    else
                        max_said = b_dest.best_now;
                    if(type_retour(-1, DateTime(working_date,data.pt_data.stop_times[stid].arrival_time%86400)) > max_said) {
                        retour[count][said]  = type_retour(stid, embarquement, DateTime(working_date, data.pt_data.stop_times[stid].arrival_time%86400));
                        best[said] = type_retour(stid, DateTime(working_date, data.pt_data.stop_times[stid].arrival_time%86400));
                        b_dest.ajouter_best_reverse(said, type_retour(stid, DateTime(working_date, data.pt_data.stop_times[stid].arrival_time%86400)));
                        if(std::find(marked_stop.begin(), marked_stop.end(), said) == marked_stop.end()) {
                            marked_stop.push_back(said);
                        }
                    }
                }

                //Si on peut arriver plus tôt à l'arrêt en passant par une autre route
                if(retour[count-1].count(said) > 0)  {
                    if(retour[count-1][said].dt >= DateTime(working_date, temps_depart)){
                        std::tie(et_temp, pam) = tardiest_trip(route, said, retour, count);
                        if(et_temp >=0) {

                            t = et_temp;
                            working_date = retour[count -1][said].dt.date;
                            stid = data.pt_data.vehicle_journeys[t].stop_time_list[i];
                            embarquement = said;
                            if(pam /*|| data.pt_data.stop_times.at(stid).arrival_time > 86400*/)
                                --working_date;
                        }
                    }
                }
                if(stid != -1)
                    prev_temps = data.pt_data.stop_times[stid].arrival_time % 86400;
            }
        }


        std::vector<unsigned int> marked_stop_copy;
        marked_stop_copy = marked_stop;
        BOOST_FOREACH(auto stop_area, marked_stop_copy) {
            std::cout << "J'inspecte : " << stop_area << std::endl;
            BOOST_FOREACH(auto stop_p, data.pt_data.stop_areas.at(stop_area).stop_point_list) {
                auto it_fp = foot_path.find(stop_p);
                if(it_fp != foot_path.end()) {
                    BOOST_FOREACH(auto connection_idx, (*it_fp).second) {
                        std::cout << "trouve : " << stop_area << std::endl;

                        unsigned int saiddest = data.pt_data.stop_points.at(data.pt_data.connections[connection_idx].destination_stop_point_idx).stop_area_idx;

                        if(best.count(saiddest) > 0)
                            best[saiddest] = std::max(best[saiddest], type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt - data.pt_data.connections[connection_idx].duration), connection));
                        else
                            best[saiddest] = type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt - data.pt_data.connections[connection_idx].duration), connection);
                        if(retour[count].count(saiddest) > 0)
                            retour[count][saiddest] = std::max(retour[count][saiddest], type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt - data.pt_data.connections[connection_idx].duration), connection));
                        else
                            retour[count][saiddest] = type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt - data.pt_data.connections[connection_idx].duration), connection);
                        b_dest.ajouter_best(saiddest, type_retour(data.pt_data.stop_times.size(), stop_area, DateTime(retour[count][stop_area].dt - data.pt_data.connections[connection_idx].duration), connection));
                        marked_stop.push_back(saiddest);
                    }
                }
            }
        }



        ++count;
    }

}


Path monoRAPTOR::makeBestPath(const map_retour_t &retour, const map_int_pint_t &best, map_int_pint_t departs, unsigned int destination_idx, unsigned int count) {
    unsigned int countb = 1;
    for(;countb<=count;++countb) {
        if(retour.at(countb).count(destination_idx) > 0) {
            if(retour.at(countb).at(destination_idx).stid == best.at(destination_idx).stid) {
                break;
            }
        }
    }

    return makePath(retour, best, departs, destination_idx, countb);
}

std::vector<Path> RAPTOR::makePathes(const map_retour_t &retour, const map_int_pint_t &best, map_int_pint_t departs, best_dest &b_dest, unsigned int count) {
    std::vector<Path> result;

    for(unsigned int i=1;i<count;++i) {
        BOOST_FOREACH(auto dest, b_dest.map_date_time) {
            if(retour.count(i) > 0) {
                if(retour.at(i).count(dest.first) > 0) {
                    if(retour.at(i).at(dest.first) != type_retour()) {
                        result.push_back(makePath(retour, best, departs, dest.first, i));
                    }
                }
            }
        }
    }

    return result;
}

Path RAPTOR::makePath(const map_retour_t &retour, const map_int_pint_t &best, map_int_pint_t departs, unsigned int destination_idx, unsigned int countb) {
    Path result;
    unsigned int current_said = destination_idx;

    type_retour r = retour.at(countb).at(current_said);
    DateTime workingDate = r.dt;
    navitia::type::StopTime current_st, prec_st;
    int said_embarquement = -1;

    bool stop = false;
    BOOST_FOREACH(auto item, departs) {
        stop = stop || (item.first == (int)current_said);
    }

    while(!stop) {
        bool debut = false, footpath = false;

        if(retour.count(countb) > 0) {
            if(retour.at(countb).count(current_said) > 0) {
                if(retour.at(countb).at(current_said).type == vj) {
                    if(said_embarquement == -1) {
                        r = retour.at(countb).at(current_said);
                        said_embarquement = r.said_emarquement;
                        current_st = data.pt_data.stop_times.at(r.stid);
                        workingDate = r.dt;
                        workingDate.normalize();
                        debut = true;
                    }
                } else {
                    if(said_embarquement == -1) {
                        r = retour.at(countb).at(current_said);
                        workingDate = r.dt;
                        workingDate.normalize();
                        result.items.push_back(PathItem(current_said, workingDate.hour, workingDate.date, -1));
                        current_said = r.said_emarquement;
                        said_embarquement = -1;
                        footpath = true;
                    }
                }
            }
        }
        if(!footpath) {
            if(!debut) {
                prec_st = current_st;
                current_st = data.pt_data.stop_times.at(data.pt_data.vehicle_journeys.at(current_st.vehicle_journey_idx).stop_time_list.at(current_st.order-1));
                if(current_st.arrival_time%86400 > prec_st.arrival_time%86400 && prec_st.vehicle_journey_idx!=navitia::type::invalid_idx)
                    --workingDate.date;
                workingDate.hour = current_st.arrival_time;
                workingDate.normalize();
            }
            current_said = data.pt_data.stop_points.at(data.pt_data.route_points.at(current_st.route_point_idx).stop_point_idx).stop_area_idx;
            if(said_embarquement == (int)current_said) {
                --countb;
                ++ result.nb_changes;
                said_embarquement = -1;
                result.items.push_back(PathItem(current_said, current_st.departure_time, workingDate.date,
                                                data.pt_data.routes.at(data.pt_data.route_points.at(current_st.route_point_idx).route_idx).line_idx));
            } else {
                result.items.push_back(PathItem(current_said, workingDate.hour, workingDate.date,
                                                data.pt_data.routes.at(data.pt_data.route_points.at(current_st.route_point_idx).route_idx).line_idx));
            }
            //            std::cout << result << std::endl;

        }

        BOOST_FOREACH(auto item, departs) {
            stop = stop || (item.first == (int)current_said);
        }
    }
    std::reverse(result.items.begin(), result.items.end());


    result.duration = result.items.back().time - result.items.front().time;
    if(result.items.back().day != result.items.front().day)
        result.duration += (result.items.back().day - result.items.front().day) * 86400;

    int count_visites = 0;
    BOOST_FOREACH(auto t, best) {
        if(t.second.stid != -1) {
            ++count_visites;
        }
    }
    result.percent_visited = 100*count_visites / data.pt_data.stop_areas.size();

    return result;
}


Path reverseRAPTOR::makePath(const map_retour_t &retour, const map_int_pint_t &best, map_int_pint_t departs, unsigned int destination_idx, unsigned int countb) {
    Path result;
    unsigned int current_said = destination_idx;

    type_retour r = retour.at(countb).at(current_said);
    DateTime workingDate = r.dt;
    navitia::type::StopTime current_st, prec_st;
    int said_embarquement = -1;

    bool stop = false;
    BOOST_FOREACH(auto item, departs) {
        stop = stop || (item.first == (int)current_said);
    }

    while(!stop) {
        bool debut = false, footpath = false;

        if(retour.count(countb) > 0) {
            if(retour.at(countb).count(current_said) > 0) {
                if(retour.at(countb).at(current_said).type == vj) {
                    if(said_embarquement == -1) {
                        r = retour.at(countb).at(current_said);
                        said_embarquement = r.said_emarquement;
                        current_st = data.pt_data.stop_times.at(r.stid);
                        workingDate = r.dt;
                        workingDate.hour = current_st.departure_time;
                        workingDate.normalize();
                        debut = true;
                    }
                } else {
                    if(said_embarquement == -1) {
                        r = retour.at(countb).at(current_said);
                        workingDate = r.dt;
                        workingDate.normalize();
                        result.items.push_back(PathItem(current_said, workingDate.hour, workingDate.date, -1));
                        current_said = r.said_emarquement;
                        said_embarquement = -1;
                        footpath = true;
                    }
                }
            }
        }
        if(!footpath) {
            if(!debut) {
                prec_st = current_st;
                current_st = data.pt_data.stop_times.at(data.pt_data.vehicle_journeys.at(current_st.vehicle_journey_idx).stop_time_list.at(current_st.order+1));
                if(current_st.arrival_time%86400 < prec_st.arrival_time%86400 && prec_st.vehicle_journey_idx!=navitia::type::invalid_idx)
                    --workingDate.date;
                workingDate.hour = current_st.arrival_time;
                workingDate.normalize();
            }
            current_said = data.pt_data.stop_points.at(data.pt_data.route_points.at(current_st.route_point_idx).stop_point_idx).stop_area_idx;
            if(said_embarquement == (int)current_said) {
                --countb;
                ++ result.nb_changes;
                said_embarquement = -1;
                result.items.push_back(PathItem(current_said, current_st.arrival_time, workingDate.date,
                                                data.pt_data.routes.at(data.pt_data.route_points.at(current_st.route_point_idx).route_idx).line_idx));
            } else {
                result.items.push_back(PathItem(current_said, workingDate.hour, workingDate.date,
                                                data.pt_data.routes.at(data.pt_data.route_points.at(current_st.route_point_idx).route_idx).line_idx));
            }
            //            std::cout << result << std::endl;

        }

        BOOST_FOREACH(auto item, departs) {
            stop = stop || (item.first == (int)current_said);
        }
    }


    result.duration = result.items.back().time - result.items.front().time;
    if(result.items.back().day != result.items.front().day)
        result.duration += (result.items.back().day - result.items.front().day) * 86400;

    int count_visites = 0;
    BOOST_FOREACH(auto t, best) {
        if(t.second.stid != -1) {
            ++count_visites;
        }
    }
    result.percent_visited = 100*count_visites / data.pt_data.stop_areas.size();

    return result;
}


Path communRAPTOR::compute(idx_t departure_idx, idx_t destination_idx, int departure_hour, int departure_day) {
    map_int_pint_t departs, destinations;
    departs[departure_idx] = type_retour(-1, DateTime(departure_day, departure_hour));
    destinations[destination_idx] = type_retour();
    return compute_raptor(departs, destinations);
}

Path communRAPTOR::compute(const type::GeographicalCoord & departure, double radius, idx_t destination_idx, int departure_hour, int departure_day) {
    map_int_pint_t bests, destinations;
    {
        Timer t("Recherche des stations de départ");

        typedef std::vector< std::pair<idx_t, double> > retour;
        retour prox;
        try {
            prox = (retour) (data.street_network.find_nearest(departure, data.pt_data.stop_area_proximity_list, radius));
        } catch(NotFound) {return Path();}
        BOOST_FOREACH(auto item, prox) {
            int temps = departure_hour + (item.second / 80);
            int day;
            if(temps > 86400) {
                temps = temps % 86400;
                day = departure_day + 1;
            } else {
                day = departure_day;
            }
            bests[item.first] = type_retour(-1, DateTime(day, temps));
        }
    }
    destinations[destination_idx] = type_retour();

    return compute_raptor(bests, destinations);
}

Path communRAPTOR::compute(const type::GeographicalCoord & departure, double radius_depart, const type::GeographicalCoord & destination, double radius_destination
                           , int departure_hour, int departure_day) {
    std::cout << "Raptor Geo geo depaart :" << departure  << " "<< radius_depart <<  " arrivee " << destination << " " << radius_destination <<" " << departure_hour << " " << departure_day <<  std::endl;
    map_int_pint_t departs, destinations;


    typedef std::vector< std::pair<idx_t, double> > retour;
    retour prox;

    try {
        prox = (retour) (data.street_network.find_nearest(departure, data.pt_data.stop_area_proximity_list, radius_depart));
    } catch(NotFound) {std::cout << "Not found 1 " << std::endl; return Path();}


    BOOST_FOREACH(auto item, prox) {
        int temps = departure_hour + (item.second / 80);
        int day;
        if(temps > 86400) {
            temps = temps % 86400;
            day = departure_day + 1;
        } else {
            day = departure_day;
        }
        departs[item.first] = type_retour(-1, DateTime(day, temps), 0, (item.second / 80));
    }


    prox.clear();
    try {
        prox = (retour) (data.street_network.find_nearest(destination, data.pt_data.stop_area_proximity_list, radius_destination));
    } catch(NotFound) {std::cout << "Not found 2 " << std::endl;return Path();}
    BOOST_FOREACH(auto item, prox) {
        std::cout << item.first << " " << item.second << std::endl;
        destinations[item.first] = type_retour((int)(item.second/80));
    }
    std::cout << "Nb stations departs : " << departs.size() << " destinations : " << destinations.size() << std::endl;

    Path result = compute_raptor(departs, destinations);
    std::cout << "Taille reponse :  " << result.items.size();
    return result;
}


}}}
